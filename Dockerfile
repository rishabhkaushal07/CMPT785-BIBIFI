FROM gcc:latest

# copy your private key into the container for ssh access to github
COPY id_rsa /root/.ssh/

# Install OpenSSL development packages
RUN apt-get update && \
    apt-get install -y libssl-dev && \
    apt-get install -y git && \
    apt-get install -y vim

WORKDIR /root/bibifi

# git clone git@github.com:rishabhkaushal07/CMPT785-BIBIFI.git
# g++ -o <> <> -lssl -lcrypto
