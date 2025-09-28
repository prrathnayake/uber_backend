#!/usr/bin/env bash
set -euo pipefail

# Smoke tests for the UserManager HTTP API. The script exercises the
# main CRUD flow exposed by the service so regressions are detected
# before committing.

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
TMP_DIR=$(mktemp -d "${TMPDIR:-/tmp}/usermanager-workflow.XXXXXX")
trap 'rm -rf "${TMP_DIR}"' EXIT

USER_SERVICE_HOST=${USER_SERVICE_HOST:-localhost}
USER_SERVICE_PORT=${USER_SERVICE_PORT:-8081}
BASE_URL="http://${USER_SERVICE_HOST}:${USER_SERVICE_PORT}"

log() {
    printf '[%s] %s\n' "$(date '+%Y-%m-%d %H:%M:%S')" "$*"
}

fail() {
    log "ERROR: $*"
    exit 1
}

require_command() {
    local cmd=$1
    if ! command -v "$cmd" >/dev/null 2>&1; then
        fail "Required command '$cmd' is not available on PATH"
    fi
}

require_command curl
require_command python3

# Best-effort TCP probe that waits for the HTTP port to open before
# issuing requests. This keeps the script from failing with connection
# refused errors when the service is still booting.
wait_for_service() {
    local host=$1
    local port=$2
    local retries=${3:-30}
    local delay=${4:-1}

    for ((attempt = 1; attempt <= retries; ++attempt)); do
        if bash -c ">/dev/tcp/${host}/${port}" 2>/dev/null; then
            log "Connection to ${host}:${port} succeeded."
            return 0
        fi
        sleep "$delay"
    done

    fail "Timed out waiting for ${host}:${port} to accept connections"
}

# Runs curl, captures the HTTP status code, and ensures the response body
# is materialised on disk before returning the path to the saved payload.
curl_json() {
    local name=$1
    local method=$2
    local url=$3
    local expected_status=$4
    local data=${5-}
    local header=${6-}

    local output="${TMP_DIR}/${name}.json"
    local -a args=(-sS -X "$method" "$url" -o "$output" -w '%{http_code}')

    if [[ -n "$data" ]]; then
        args+=(-H 'Content-Type: application/json' --data "$data")
    fi

    if [[ -n "$header" ]]; then
        args+=(-H "$header")
    fi

    local http_code
    local curl_status

    set +e
    http_code=$(curl "${args[@]}")
    curl_status=$?
    set -e

    if (( curl_status != 0 )); then
        [[ -f "$output" ]] && log "Response body for ${name}:" && sed 's/^/    /' "$output"
        fail "curl exited with status ${curl_status} for ${method} ${url}"
    fi

    if [[ "$http_code" != "$expected_status" ]]; then
        if [[ -f "$output" ]]; then
            log "Response body for ${name}:"
            if [[ -s "$output" ]]; then
                sed 's/^/    /' "$output"
            else
                log "    <empty>"
            fi
        else
            log "No response file created for ${name}."
        fi
        fail "Unexpected HTTP status ${http_code} (expected ${expected_status}) for ${method} ${url}"
    fi

    if [[ ! -f "$output" ]]; then
        fail "Expected response file ${output} was not created"
    fi

    if [[ -s "$output" ]]; then
        log "${name} -> HTTP ${http_code}"
        sed 's/^/    /' "$output"
    else
        log "${name} -> HTTP ${http_code} (empty body)"
    fi

    printf '%s\n' "$output"
}

# Reads a JSON field from the supplied document. Nested keys can be
# referenced using dot notation and numeric indices (e.g. `0.user_id`).
json_get() {
    local file=$1
    local path=$2

    python3 - "$file" "$path" <<'PY'
import json
import sys

file_path, query = sys.argv[1], sys.argv[2]
with open(file_path, 'r', encoding='utf-8') as handle:
    data = json.load(handle)

value = data
for part in query.split('.'):
    if isinstance(value, list):
        try:
            index = int(part)
        except ValueError as exc:
            raise SystemExit(f"Expected numeric index in path component '{part}'") from exc
        try:
            value = value[index]
        except IndexError as exc:
            raise SystemExit(f"List index {index} out of range while resolving '{query}'") from exc
    elif isinstance(value, dict):
        if part not in value:
            raise SystemExit(f"Key '{part}' missing while resolving '{query}'")
        value = value[part]
    else:
        raise SystemExit(f"Cannot descend into '{part}' because current value is not a list or dict")

if isinstance(value, bool):
    print('true' if value else 'false')
else:
    print(value)
PY
}

urlencode() {
    python3 -c 'import sys, urllib.parse; print(urllib.parse.quote_plus(sys.argv[1]))' "$1"
}

wait_for_service "$USER_SERVICE_HOST" "$USER_SERVICE_PORT"

log "Beginning UserManager workflow smoke test against ${BASE_URL}"

suffix=$(date '+%Y%m%d%H%M%S')
username="workflow.user.${suffix}"
email="${username}@example.com"
password_initial="SecretPass!1"
password_new="SecretPass!2"

availability_file=$(curl_json "user-availability" GET "${BASE_URL}/user/exists?username=$(urlencode "$username")&email=$(urlencode "$email")" 200)
[[ $(json_get "$availability_file" 'usernameTaken') == "false" ]] || fail "Username ${username} is unexpectedly marked as taken"
[[ $(json_get "$availability_file" 'emailTaken') == "false" ]] || fail "Email ${email} is unexpectedly marked as taken"

read -r -d '' signup_payload <<'PAYLOAD'
{
  "firstName": "Workflow",
  "middleName": "QA",
  "lastName": "User",
  "countryCode": "+1",
  "mobileNumber": "555000${suffix: -4}",
  "address": "1 Integration Way",
  "email": "${email}",
  "username": "${username}",
  "password": "${password_initial}",
  "role": "rider",
  "preferredLanguage": "en",
  "currency": "USD",
  "country": "US"
}
PAYLOAD

signup_file=$(curl_json "user-signup" POST "${BASE_URL}/signup" 201 "$signup_payload")
[[ $(json_get "$signup_file" 'message') == "Signup successful" ]] || fail "Unexpected signup response"

search_file=$(curl_json "user-search" GET "${BASE_URL}/user/search?username=$(urlencode "$username")" 200)
created_user_id=$(json_get "$search_file" '0.user_id')
[[ -n "$created_user_id" ]] || fail "Failed to resolve user id for ${username}"
log "Created user id: ${created_user_id}"

read -r -d '' login_payload <<'PAYLOAD'
{
  "username": "${username}",
  "password": "${password_initial}"
}
PAYLOAD

login_file=$(curl_json "user-login" POST "${BASE_URL}/login" 200 "$login_payload")
user_id_from_login=$(json_get "$login_file" 'user_id')
[[ "$user_id_from_login" == "$created_user_id" ]] || fail "Login returned unexpected user id ${user_id_from_login}"
token=$(json_get "$login_file" 'token')
[[ -n "$token" ]] || fail "Login did not return a JWT token"

me_file=$(curl_json "user-me" GET "${BASE_URL}/user/me" 200 "" "Authorization: Bearer ${token}")
[[ $(json_get "$me_file" 'user_id') == "$created_user_id" ]] || fail "JWT introspection returned mismatched user id"

read -r -d '' update_payload <<'PAYLOAD'
{
  "address": "42 Updated Avenue",
  "country": "US"
}
PAYLOAD

update_file=$(curl_json "user-update" PUT "${BASE_URL}/user/${created_user_id}" 200 "$update_payload")
[[ $(json_get "$update_file" 'message') == "User updated successfully" ]] || fail "Unexpected response while updating user"

read -r -d '' profile_payload <<'PAYLOAD'
{
  "preferredLanguage": "en-GB",
  "vehicle_type": "hybrid"
}
PAYLOAD

profile_file=$(curl_json "user-profile-update" PATCH "${BASE_URL}/user/${created_user_id}/profile" 200 "$profile_payload")
[[ $(json_get "$profile_file" 'message') == "Profile updated" ]] || fail "Partial profile update did not succeed"

read -r -d '' password_payload <<'PAYLOAD'
{
  "oldPassword": "${password_initial}",
  "newPassword": "${password_new}"
}
PAYLOAD

password_file=$(curl_json "user-password-update" PATCH "${BASE_URL}/user/${created_user_id}/password" 200 "$password_payload")
[[ $(json_get "$password_file" 'message') == "Password updated" ]] || fail "Password change request failed"

read -r -d '' login_new_payload <<'PAYLOAD'
{
  "username": "${username}",
  "password": "${password_new}"
}
PAYLOAD

login_new_file=$(curl_json "user-login-new-password" POST "${BASE_URL}/login" 200 "$login_new_payload")
[[ $(json_get "$login_new_file" 'user_id') == "$created_user_id" ]] || fail "Login with new password failed"

delete_file=$(curl_json "user-delete" DELETE "${BASE_URL}/user/${created_user_id}" 200)
[[ $(json_get "$delete_file" 'message') == "User deleted successfully" ]] || fail "User deletion did not succeed"

log "UserManager workflow smoke test completed successfully."
