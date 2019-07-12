FROM python:slim
RUN apt-get update && apt-get upgrade -y
RUN apt-get install -y build-essential

COPY build_requirements.txt /root/requirements.txt

RUN pip3 install --upgrade pip
RUN pip install -r /root/requirements.txt

COPY . /root
WORKDIR /root
RUN cd /root && make install

RUN apt-get install -y gdb

ENTRYPOINT ["/bin/bash"]