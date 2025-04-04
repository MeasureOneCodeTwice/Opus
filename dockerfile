FROM alpine:3.14
ARG directory

COPY dependencies /project/dependencies
RUN apk add $(cat project/dependencies)

COPY . /project
COPY tests/* /project/tests/


CMD ["make", "-C $directory run"]
