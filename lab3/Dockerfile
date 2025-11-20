FROM alpine:3.18

RUN apk add --no-cache gcc strace musl-dev

WORKDIR /app
COPY . .

RUN gcc -o parent parent.c -lrt -lpthread && \
    gcc -o child1 child1.c -lrt -lpthread && \
    gcc -o child2 child2.c -lrt -lpthread

CMD ["strace", "-f", "./parent"]