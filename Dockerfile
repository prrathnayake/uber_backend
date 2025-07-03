# Dockerfile
FROM cpp_base:1.0

# Set working directory
WORKDIR /UberBackend

# Copy all your source code into container
COPY . .

# Add and set entrypoint
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

EXPOSE 8081

ENTRYPOINT ["/entrypoint.sh"]