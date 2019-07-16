workflow "Build & Test Pytubes" {
  on = "push"
  resolves = ["Update Docs"]
}

action "Versioned" {
  uses = "./.github/actions/sh"
  args = "python tools/scripts/update-version.py"
}

action "Run Tests" {
  uses = "./"
  needs = ["Versioned"]
  args = "py3 make test"
}

action "Build Docs" {
  uses = "./"
  needs = ["Versioned"]
  args = "py3 make doc"
}

action "Tests Complete" {
  uses = "./.github/actions/checkpoint"
  needs = ["Run Tests", "Build Docs"]
}

action "Build wheel 3.5" {
  uses = "./"
  args = "make-wheel cp35-cp35m"
  needs = ["Tests Complete"]
}

action "Build wheel 3.6" {
  uses = "./"
  args = "make-wheel cp36-cp36m"
  needs = ["Tests Complete"]
}

action "Build wheel 3.7" {
  uses = "./"
  args = "make-wheel cp37-cp37m"
  needs = ["Tests Complete"]
}

action "Build Complete" {
  uses = "./.github/actions/checkpoint"
  needs = ["Build wheel 3.5", "Build wheel 3.6", "Build wheel 3.7"]
}

action "Test wheelhouse" {
  uses = "./"
  args = "ls /github/workspace/wheelhouse"
  needs = ["Build Complete"]
}

action "If Tag" {
  uses = "actions/bin/filter@master"
  args = "tag"
  needs = ["Build Complete"]
}

action "Update Docs" {
  uses = "./" 
  args = "update-docs"
  needs = ["If Tag"]
  secrets = ["ACCESS_TOKEN"]
}

action "Deploy Complete" {
  uses = "./.github/actions/checkpoint"
  needs = ["Update Docs"]
}
