FROM quay.io/pypa/manylinux2010_x86_64

RUN yum install -y rh-python35-python-devel

COPY build_requirements.txt /tmp/requirements.txt

RUN (. /opt/rh/rh-python35/enable && pip install --upgrade pip && pip install -r /tmp/requirements.txt)

COPY . /pristine
WORKDIR /pristine

RUN (. /opt/rh/rh-python35/enable && make clean)

ENTRYPOINT ["tools/entrypoint.sh"]

LABEL "com.github.actions.name"="Run action using the pytubes build docker environment"
LABEL "com.github.actions.description"=""
LABEL "com.github.actions.icon"="share-2"
LABEL "com.github.actions.color"="blue"

LABEL "repository"="http://github.com/stestagg/pytubes"
LABEL "homepage"="http://github.com/stestagg/pytubes"
LABEL "maintainer"="Stestagg <stestagg@gmail.com>"