FROM alpine:3.18

RUN apk add --no-cache gcc strace musl-dev

WORKDIR /app
COPY . .

RUN gcc -o parent parent.c && \
    gcc -o child1 child1.c && \
    gcc -o child2 child2.c

# Запускаем сразу с strace
CMD ["strace", "-f", "./parent"]