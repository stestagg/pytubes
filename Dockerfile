FROM python:slim
RUN apt-get update && apt-get upgrade -y
RUN apt-get install -y build-essential

COPY . /root

RUN pip3 install --upgrade pip
COPY build_requirements.txt /root/requirements.txt
RUN pip install -r /root/build_requirements.txt

COPY . /root
WORKDIR /root
RUN cd /root && make test

ENTRYPOINT ["/bin/bash"]