000 0
002 1
004 3
006 "Loop exit."
017 "\n"
$ 1 19
# 6             ; x -> /-2, tmp2 -> /-4, tmp4 -> /-6
i 000 /-2
l /-2 004 6     ; Check if x has exceed 3
i 000 /-4       ; If no, then set tmp2 to 1 (true)
j 7
i 002 /-4       ; If yes, then set tmp2 to 0 (false)
e /-4 000 16    ; If tmp2 is false, then exit the loop
p #/-2          ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
c 0 -9
^ 2
p #017          ; This is the loop body.  Print x, newline, and increment.
c 0 -11         ; After incrementation, jump back to line 3 to recalculate
^ 2             ; whether the condition (x < 3) still holds.
a /-2 002 /-2
j 3             ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
p #006          ;
c 0 -11         ; Loop has exited, print "Loop exit." message and newline.
^ 2
p #017
c 0 -11
^ 2
h               ; Program ends.
;
;			This VMQ program is based off the following simple C++ program:
;
;			int main()
;			{
;				int x;
;				x = 0;
;				while (x < 3)
;				{			
;					cout << x << endl;
;					x += 1;
;				}
;				
;				cout << "Loop exit." << endl;
;
;				return 0;
;			}
;
