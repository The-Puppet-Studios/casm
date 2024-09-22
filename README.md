# CASM <img src="logo.png" alt="Logo" style="float: right; width: 100px;"/>
CASM is a high-level assembly inspired interpreted language designed to make working with registers easier and more high-level. It supports things like printing, and other stuff.

# Documentation
## Out
To print something you can use the out function. Heres an example:

```
out "Welcome to CASM!";
```

## Variables
There are 3 datatypes right now. str, int, and sml. sml can be either 0 or 1 and it stands for small.

Heres an example of making a variable:

```
int coolint = 5;
str coolstring = "5";
sml smallint = 1;
```

## If Else
To do an if else you need to define a syntax like this:

```
int cooler = 5;

if cooler == 5
    out "Nice!";
else
    out "Nope.";
end
```

## Comments
To add a comment you need to put a hashtag at the beginning of the line. Like this:

```
# This wont be interpreted!
```

## Input
To make an input variable you need to add in to the beginning.

Also for input variables dont add an equals sign. It considers the = as part of the prompt.

That will be fixed at some point.

Anyways with input variables if you define them as int for example the user has to put in an integar.

You make an input variable like this:

```
in str input "Input a string: ";
in int intinput "Input an integar: ";
in sml smlinput "Input a small integar (1 or 0): ";
```

# Credits
Voltaged: [Github](https://github.com/VoltagedDebunked)

# Honorable Mentions
CASM++: [Github](https://github.com/Volis-Tech-Organization/CasmPlusPlus)