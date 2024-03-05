# TODO: move from python2 to python3
FROM python:2

USER root

WORKDIR /deckard

RUN apt-get update -y

RUN apt-get update -y && apt-get install -y \
	git \
	make \
	gcc \
	g++ \
	cmake \
	libtool \
	autoconf \
	automake \
	libltdl-dev \
	bison \
	flex

COPY . .

WORKDIR /deckard/src/main

RUN ./build.sh

