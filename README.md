# oh-come-on
Simple ptrace-based golang `compile` process tracer disabling the annoying `declared and not used` and `imported and not used` errors.

Does not touch disk nor modifies go sources. No rebuild of go compiler required.

Tested with [go 1.24.1](https://archlinux.org/packages/extra/x86_64/go/) as well as local builds of go compiler. Should work on a variety of go versions.

## build
`make`

## sample usage
![image](https://github.com/user-attachments/assets/d089a972-c585-4cfc-af4d-9c03337cb8e2)
