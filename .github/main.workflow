workflow "Build & Test Pytubes" {
  on = "push"
  resolves = ["Update Docs"]
}

action "Update Docs" {
  uses = "./"
  args = "update-docs"
  needs = ["Versioned"]
  secrets = ["ACCESS_TOKEN"]
}

action "Versioned" {
  uses = "./.github/actions/sh"
  args = "python tools/update_version.py"
}

action "Run Tests" {
  uses = "./"
  needs = ["Versioned"]
  args = "set"
}

action "Build Docs" {
  uses = "./"
  needs = ["Versioned"]
  args = "py3 make doc"
}

action "Tests Complete" {
  uses = "actions/bin/sh@master"
  needs = ["Run Tests", "Build Docs"]
  args = ["true"]
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
  uses = "actions/bin/sh@master"
  needs = ["Build wheel 3.5", "Build wheel 3.6", "Build wheel 3.7"]
  args = ["true"]
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

action "Deploy Docs" {
  uses = "./.github/actions/update_docs"
  needs = ["Build Complete", "If Tag"]
  env = {
    BRANCH = "gh-pages"
    FOLDER = "docs/_build/html"
  }
  secrets = ["ACCESS_TOKEN"]
}

action "Deploy Complete" {
  uses = "actions/bin/sh@master"
  needs = ["Deploy Docs"]
  args = ["true"]
}
