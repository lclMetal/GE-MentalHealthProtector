# GE-MentalHealthProtector
A script combiner for working on multiple-script-file projects with Game Editor

## Usage
TODO: Add usage instructions. (Yes, it was important to prioritize the wall-of-text
of incoherent ramblings of _what_ the program is made for.)

## What on earth is this?
Game Editor Mental Health Protector is a simple tool designed to 
"protect the mental health" of both, the developer working on a
Game Editor project _and_ Game Editor itself.

## That didn't help, what the NULL is this?
Okay, here's a more descriptive description.

Game Editor isn't exactly friendly for developers when it comes to
large projects with multiple long script files. The order in which
Game Editor processes the "pages" in Global Code isn't really
defined anywhere. That easily leads to situations where a program
symbol defined on one page causes another page to break if Game
Editor processes the latter page first.

So, any dependencies between multiple pages have the potential to disturb
Game Editor, resulting in random crashes, cryptic error messages and
unexpected differences in behavior between testing in Game Editor's "Game mode"
and running an exported build of the game.

These problems can cause the developer to experience deteriorating mental
health, including symptoms such as paranoia, developing superstitions,
and general degradation of mood and quality of life.

## Ookay.. but what does it do?
A workaround to the problems described above is to combine all the scripts
to one long script. But that comes with the obvious disadvantage of having
to work with a potentially very large code file. The program aims to solve
the problems while still allowing the developer to split their code in
multiple files.

The system monitors a given directory, the "source folder", for any
file changes and when it detects a change, it combines all the scripts
to one unified script file which can then be used in Game Editor.
