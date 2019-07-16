workflow "Build & Test Pytubes" {
  on = "push"
  resolves = ["Build & Test"]
}

action "Build Image" {
  uses = "actions/docker/cli@86ff551d26008267bb89ac11198ba7f1d807b699"
  args = "build -t stestagg/pytubes ."
}

action "Build & Test" {
  uses = "actions/docker/cli@86ff551d26008267bb89ac11198ba7f1d807b699"
  needs = ["Build Image"]
  args = "run --rm stestagg/pytubes -c 'make test'"
}

action "Make Docs" {
  uses = "actions/docker/cli@86ff551d26008267bb89ac11198ba7f1d807b699"
  needs = ["Build Image"]
  args = "run --rm stestagg/pytubes -c 'make doc'"
}
