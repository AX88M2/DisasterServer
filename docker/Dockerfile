FROM ubuntu:latest

WORKDIR /app

COPY config.toml .
COPY DisasterServerCXX .
COPY database.db .
COPY launch.sh .

RUN chmod +x launch.sh .

EXPOSE 8606/udp

CMD ["./launch.sh"]
