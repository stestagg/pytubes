FROM quay.io/pypa/manylinux2014_x86_64

RUN yum install -y rh-python38-python-devel rh-python38-python-pip

COPY build_requirements.txt /tmp/requirements.txt

RUN (. /opt/rh/rh-python38/enable && python -m pip install --upgrade pip && python -m pip install -r /tmp/requirements.txt)

COPY . /pristine
WORKDIR /pristine

RUN (. /opt/rh/rh-python38/enable && make clean)

ENTRYPOINT ["tools/entrypoint.sh"]

LABEL "com.github.actions.name"="Run action using the pytubes build docker environment"
LABEL "com.github.actions.description"=""
LABEL "com.github.actions.icon"="share-2"
LABEL "com.github.actions.color"="blue"

LABEL "repository"="http://github.com/stestagg/pytubes"
LABEL "homepage"="http://github.com/stestagg/pytubes"
LABEL "maintainer"="Stestagg <stestagg@gmail.com>"
