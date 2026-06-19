# Strict and necessary rules for agents.
- The contents of this file will NOT be avoided and will be used when formulating a response.

## Basic project description:
- Simple, project meant for fun and studying C.
- The project revolves around making a real time calculator CLI tool, by reading user input, tokenizing input, and calculating based off of the tokens.
- Focuses on good practices and having algorithms that aren't unecessarily complex.

## Design decisions:
- Naming scheme with variables such as 'isVariable' or 'coolFunctionName' and simple and immediately understandable macros or names.
- Avoiding memory allocation as much as possible - if stack can be used, use it over memory allocation any time.
- Taking user input trough non-canonical terminal mode, putting it onto a buffer (after validating input), and eventually tokenizing it and calculating. Inputs such as arrow keys, CTRL+A or CTRL+E, or CTRL+X are all processed, and the more shell-like the tool becomes, the merrier.
- Splitting a job across multiple functions, instead of leaving everything to a single function.
- Reusing assets as much as possible, and if possible, slightly tweak those to fit a new situation + the old one.
- Understandable and consistent error messages.
- Making a nice and simple UX.

## AI Agent guidelines:
- AI agents will NEVER write or implement code on their own - you're here to help the user and that's it, the user will ALWAYS be the one to write code, not you.
- You will explain your code, and if necessary, ask design questions to the user. 
- If the question seems absurd, or getting to the response is impossible or unbearably difficult, you need to rethink or ask the user if they really asked correctly.
- You're here to teach and answer questions asked by the user, with correct information.
- You will always follow these guidelines.
- Whenever you're going to check the code file, you will apply the following algorithm. In your context, you will store the output of `stat -c %Y` for every file in the project (do this in the start of a session). Everytime you need to re-check the file's contents, or when the user asks about the file, you will run the command again on the file, and compare the output to what is in your context. If the values match, don't update context, but if the values don't match, you will update context for the file. Situations where you need to check with the command, for example, are when the user asks about a specific part of the code, or when you're studying the project. Note that the only exceptions for value storage are files such as binaries, as they aren't supposed to be checked.

## Current implementations (list might be outdated)
- Basic actions like `x+(y*z)`.
- Addition (+), subtraction (-), multiplication (*), division (/), powers (^) and square roots (sqrt[x]).
- Arrow keys to navigate input.
- CTRL+A to moving to start of input, and CTRL+E to move to end of input.
- CTRL+X to clear all input.
- CTRL+W to delete word.
- Basic error handling + buffer validation before calculation.
