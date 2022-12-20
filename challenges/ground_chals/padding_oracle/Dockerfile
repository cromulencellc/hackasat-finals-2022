FROM python:3.10-slim

RUN pip3 install pycryptodome

EXPOSE 8000
VOLUME /keys 

WORKDIR /oracle

ENV AES_SECRET IAMTHESECRETKEYLOLOLOLOL

COPY *.py /oracle/
COPY *.yml /oracle/

CMD python3 server.py --port 8000 --hostname 0.0.0.0 --key_files /keys --aes_secret $AES_SECRET