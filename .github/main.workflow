workflow "Build & Test" {
  on = "push"
  resolves = ["Build"]
}

action "Build" {
  uses = "actions/docker/cli@8cdf801b322af5f369e00d85e9cf3a7122f49108"
  args = "run --rm -v $PWD:/tubes python:slim /tubes/docker_build.sh"
}
