# CASM
CASM is a high-level assembly inspired interpreted language designed to make working with registers easier and more high-level. It supports things like printing, and other.

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
