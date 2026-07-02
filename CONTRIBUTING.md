# Contributing to PongC

## Getting started

If you are new to the codebase, take a look at [ARCHITECTURE.md](ARCHITECTURE.md) to understand project philosophy, architecture

## Reporting problems

- [Search existing issues](https://github.com/zerfithel/pongc/issues) (including closed ones)
- Update PongC to latest version to see if problem persists
- In issues please include such information as operating system, PongC version, steps to reproduce bug, potential fixes (if you have any idea of any) and describe the error and how it affects program

## Pull Requests (PRs)

Do NOT change multiple stuff in one pull request, if you changed more than one thing then open separate pull requests for each change.

### PR Description

Your PRs description should describe fixed bug/enhancement/new feature as best as possible.

Example of good PR description:
- PR Title: `segfault_fix(main.c): added sanity checks to malloc()`
- PR Description: `Added multiple sanity checks to returned pointers by malloc() to check if memory allocation succeded, if not, handle errors to avoid segmentation fault. The following code was problematic: ...` ...

### AI-Assisted contributions

Using AI is acceptable if you review output, test it, check for edge cases etc.

## Syntax style

Format files by running:
```bash
make format # format all changed files
```

## Before opening a pull request

Before opening a new pull request run `make format` and make sure all tests pass and program works as should (test on localhost). Document your changes in code and in PR description. Always write tests to your contributions if needed. 

## Roadmap

See project [ROADMAP.md](ROADMAP.md).
