workflow "Build & Test" {
  on = "push"
  resolves = ["GitHub Action for Docker"]
}

action "Build" {
  uses = "actions/docker/cli@8cdf801b322af5f369e00d85e9cf3a7122f49108"
  args = "build -t stestagg/pytubes ."
}

action "GitHub Action for Docker" {
  uses = "actions/docker/cli@86ff551d26008267bb89ac11198ba7f1d807b699"
  needs = ["Build"]
  args = "run stestagg/pytubes make test"
}
