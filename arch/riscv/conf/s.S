.text
.global _start
_start:
li a1, 1
loop:
add a0, a0, a1
jal loop
