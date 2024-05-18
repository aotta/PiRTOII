    REM IntyColor v1.1.7 Dec/03/2018
    REM Command: intycolor -b pirto_s.bmp pirto.bas 
    REM Created: Fri Jan 19 11:47:51 2024

    REM stub for showing image

    ASM MEMATTR $8000, $9fFF, "+RWN"

    ' We need some important constants.
    INCLUDE "constants.bas"
    ' Splash screen 
    INCLUDE "constant2.bas"
    CONST mfile=$817f
    CONST mst=$813f
    const riga=$8899
    const joy=$8889
    const joyck=$8119
    const hack=$9111
    const dirfile=$8651
    const chk=$815e
    const debug=$8163
    

    dim tipo(10)
    poke(mst),0
    poke(chk),0
    poke(dirfile),0

   
    GOSUB reset_sound
    for A=1 to 2    
        IF A=1 THEN #C=477
        IF A=2 THEN #C=239
    SOUND 0,#C,PSG_ENVELOPE_ENABLE
    SOUND 1,(#C+1)/2,PSG_ENVELOPE_ENABLE
    SOUND 2,#C*2,PSG_ENVELOPE_ENABLE
    SOUND 3,6000,PSG_ENVELOPE_SINGLE_SHOT_RAMP_DOWN_AND_OFF ' Slow decay, single shot \______
    FOR C = 1 TO 30:WAIT:NEXT C
    NEXT A
    GOSUB reset_sound
    poke (joyck),0
    #cnt=0
    WHILE (cont = NO_KEY) and (peek(joyck)<>123) and (#cnt<100) ' 0x119)  
        #cnt=#cnt+1
        WAIT
    WEND
    poke(chk),0
    WAIT
   
    poke(joyck),0 '0x119
    poke (joy),0

   ' poke(joy),1 ' carica i file
  
avanti:
    
    curriga=0
    cls     ' Clear the screen.

    wait
    PRINT AT SCREENPOS(2, 0) COLOR CS_RED,"PiRTO II"
    PRINT COLOR CS_WHITE," - Flash"
    PRINT AT SCREENPOS(0, 11) COLOR CS_WHITE,"ENT/BT:sel*CLR:../"
    
   
      
menu:
    GOSUB leggimenu
    c = cont
    
    if (c=KEYPAD_ENTER) or (c=BUTTON_1) or (c=BUTTON_2) or (c=BUTTON_3) then 
        poke(joy),0:poke(joyck),0
        goto select
    end if

    if (c=KEYPAD_CLEAR) then
        sound 0,120,15
        for p=0 to 5:next p
        sound 0,0,0 
        cls
        k=0
        while peek(joyck)<>1 'and #cnt<500' 0x119
            poke(joy),5
            print at screenpos(3,2) color CS_TAN,"UP DIR" ' root
            print at screenpos(3,5),"PLEASE WAIT..."
            if k=0 then print at screenpos(10,8),BG28 + CS_BLUE
            if k=1 then print at screenpos(10,8),BG29 + CS_BLUE
            if k=2 then print at screenpos(10,8),BG30 + CS_BLUE
            if k=3 then print at screenpos(10,8),BG31 + CS_BLUE
            k=k+1:#cnt=#cnt+1
            if k>3 then k=0   
        wend
        
        poke(joyck),0
        poke (joy),0  
        goto avanti
    end if    

    if cont.down and (curriga<lastriga) then
       curriga=curriga+1 ' giù
       sound 0,140,15
       for p=0 to 9:next p
       wait
       sound 0,0,0 
    end if
    
    if cont.up and curriga>0 then 
       curriga=curriga-1 ' su
       sound 0,140,15
       for p=0 to 9:next p
       wait
       sound 0,0,0 
    end if
    if cont.right then 
        sound 0,140,15
        sound 0,0,0 
        k=0:#cnt=0
        cls
        print at screenpos(3,2) color CS_TAN,"Loading next page" ' root
        print at screenpos(3,5),"Please wait..."
        poke (joy),3
        while peek(joyck)<>1 'and #cnt<500 ' 0x119
            if k=0 then print at screenpos(10,8),BG28 + CS_BLUE
            if k=1 then print at screenpos(10,8),BG29 + CS_BLUE
            if k=2 then print at screenpos(10,8),BG30 + CS_BLUE
            if k=3 then print at screenpos(10,8),BG31 + CS_BLUE    
            k=k+1:#cnt=#cnt+1
            if k>3 then k=0
            poke(joy),3
        wend
        poke(joyck),0 '0x119
        poke (joy),0
        goto avanti
    end if
    if cont.left then 
        sound 0,120,15
        for p=0 to 9:next p
        wait
        sound 0,0,0 
        k=0:#cnt=0
        cls
        print at screenpos(3,2) color CS_TAN,"Prev DIR" ' root
        print at screenpos(3,5),"Please wait..."
        poke(joy),4
        while peek(joyck)<>1 'and #cnt<500 ' 0x119
            if k=0 then print at screenpos(10,8),BG28 + CS_BLUE
            if k=1 then print at screenpos(10,8),BG29 + CS_BLUE
            if k=2 then print at screenpos(10,8),BG30 + CS_BLUE
            if k=3 then print at screenpos(10,8),BG31 + CS_BLUE
            k=k+1:#cnt=#cnt+1
            if k>3 then k=0
            poke(joy),4
        wend
        poke(joyck),0
        poke (joy),0  
        goto avanti
    end if


    goto menu
   
select:
    k=0
    CLS
    print at screenpos(3,2) color CS_TAN," LOADING" ' root
    print at screenpos(3,5),"PLEASE WAIT..."
    'print at screenpos(3,6),<1>tipo(curriga)
    
    poke(riga),curriga+1      
    poke(joy),2
'poivia ---------------------------------------------------
    if peek(debug)>1 then gosub debug

    while peek(joyck)<>1 and #cnt<500' 0x119
        if k=0 then print at screenpos(10,8),BG28 + CS_BLUE
        if k=1 then print at screenpos(10,8),BG29 + CS_BLUE
        if k=2 then print at screenpos(10,8),BG30 + CS_BLUE
        if k=3 then print at screenpos(10,8),BG31 + CS_BLUE
    
        k=k+1:#cnt=#cnt+1
        if k>3 then k=0
 
    wend
    poke(joyck),0
    poke (joy),0  
    if tipo(curriga)=2 then goto avanti ' directory
    'goto avanti    
' sennò è Gioco
    cls
    print at screenpos(3,2) color CS_TAN," LOADING GAME" ' root
    print at screenpos(3,5),"PLEASE WAIT..."
    print at screenpos(3,6),<1>tipo(curriga)

fine:
    goto fine

leggimenu: PROCEDURE
    lastriga=0
    emptylines=0
    for j=0 to 9
        lastriga=lastriga+1
        for i=0 to 19
         
            #mem=peek((mfile+i*2)+40*j)
            if i=0 and #mem=0 then ' empty line
                #mem=32
                emptylines=emptylines+1
                tipo(j)=0
            end if 
            if j=curriga then 
                if peek($9000+j)=1 then 
                    tipo(j)=2 'dir
                else
                    tipo(j)=1 'file
                end if
                if #mem<32 then #mem=32
                PRINT AT screenpos(i,j+1),(#mem-32)*8+CS_YELLOW
            else
                if peek($9000+j)=1 then
                    tipo(j)=2 'dir
                    if #mem<32 then #mem=32
                    PRINT AT screenpos(i,j+1), (#mem-32)*8+CS_BLUE
                else
                    tipo(j)=1
                    if #mem<32 then #mem=32
                    PRINT AT screenpos(i,j+1), (#mem-32)*8+CS_GREEN
                end if  
            end if
         
        next i
    next j
    lastriga=lastriga-emptylines-1
if peek(debug)>0 then
    print at screenpos(0,11),<2>lastriga
    print at screenpos(17,11),<2>emptylines
    print at screenpos(8,11),<2>curriga
    print at screenpos(0,11),<3>peek($8000+$1030)
    print at screenpos(4,11),<3>peek($8000+$1031)
    print at screenpos(12,11),<3>peek($8000+$1032)
end if

  END

  ' 32 bitmaps
    
debug:  PROCEDURE
   cls
looppa:
 for i=0 to 20:#mem=peek(mfile+i*2):print at screenpos(i,0),(#mem-32)*8+CS_WHITE:next i
 for i=0 to 20:#mem=peek(mfile+i*2+40):print at screenpos(i,1),(#mem-32)*8+CS_WHITE:next i
 for i=0 to 20:#mem=peek(mfile+i*2+80):print at screenpos(i,2),(#mem-32)*8+CS_WHITE:next i
 print at screenpos(0,5),<2>peek(mfile+200)
print at screenpos(4,5),<6>peek(mfile+202)
print at screenpos(12,5),<6>peek(mfile+204)
print at screenpos(0,6),<6>peek(mfile+206)
print at screenpos(10,6),<6>peek(mfile+212)
print at screenpos(2,7),<2>peek(mfile+208)
print at screenpos(5,7),<2>peek(mfile+210)

if peek(debug)=2 then goto looppa
end
 

reset_sound:    PROCEDURE
    SOUND 0,1,0
    SOUND 1,1,0
    SOUND 2,1,0
    SOUND 4,,$38
    RETURN
    END


screen_bitmaps_0:
    DATA $0000,$0000,$3F00,$FF7F
    DATA $0000,$0000,$FF00,$FFFF
    DATA $0000,$0000,$0301,$0303
    DATA $0000,$0000,$FFFF,$F1FF
    DATA $0000,$0000,$F180,$FCF9
    DATA $0000,$0000,$FFFF,$1FFF
    DATA $0000,$0000,$FCFE,$80FC
    DATA $0000,$0000,$1F03,$F87F
    DATA $0000,$0000,$F0C0,$FCF8
    DATA $8CCC,$0C0C,$1C0C,$3C1C
    DATA $7070,$7070,$7070,$7170
    DATA $0703,$0707,$0F0F,$0F0F
    DATA $E1E0,$FFE3,$FFFF,$FFFF
    DATA $F8FC,$F0F8,$00E0,$8000
    DATA $1F1F,$3F1F,$3E3F,$7E3E
    DATA $0381,$0703,$0707,$0707
screen_bitmaps_1:
    DATA $F0F0,$E0F0,$E0E0,$C1C0
    DATA $7C7C,$FC7C,$FCFC,$F8F8
    DATA $7838,$3078,$0000,$0000
    DATA $7673,$1C3E,$0000,$0000
    DATA $1F1F,$3F1F,$003F,$0000
    DATA $9F9F,$8F8F,$0007,$0000
    DATA $C080,$E0E0,$00F0,$0000
    DATA $7C7C,$FC7C,$00F8,$0000
    DATA $0707,$0103,$0000,$0000
    DATA $E3C1,$FFFF,$0078,$0000
    DATA $E0F0,$00C0,$0000,$0000

    ' Real Copyright Symbol
    BITMAP ".######."
    BITMAP "#......#"
    BITMAP "#..###.#"
    BITMAP "#.#....#"
    BITMAP "#.#....#"
    BITMAP "#..###.#"
    BITMAP "#......#"
    BITMAP ".######."

BITMAP "....##.."
BITMAP "......#."
BITMAP ".......#"
BITMAP ".......#"
BITMAP "........"
BITMAP "........"
BITMAP "........"
BITMAP "........"

BITMAP "........"
BITMAP "........"
BITMAP "........"
BITMAP "........"
BITMAP ".......#"
BITMAP ".......#"
BITMAP "......#."
BITMAP "....##.."

BITMAP "........"
BITMAP "........"
BITMAP "........"
BITMAP "........"
BITMAP "#......."
BITMAP "#......."
BITMAP ".#......"
BITMAP "..##...."

BITMAP "..##...."
BITMAP ".#......"
BITMAP "#......."
BITMAP "#......."
BITMAP "........"
BITMAP "........"
BITMAP "........"
BITMAP "........"


    REM 20x12 cards
screen_cards:
    DATA $0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
    DATA $0000,$0000,$0000,$0000,$0000,$0802,$080A,$0000,$0812,$081A,$0822,$082A,$0832,$083A,$0842,$0000,$0000,$0000,$0000,$0000
    DATA $0000,$0000,$0000,$0000,$0000,$084A,$0852,$0000,$085A,$0862,$086A,$0872,$087A,$0882,$088A,$0000,$0000,$0000,$0000,$0000
    DATA $0000,$0000,$0000,$0000,$0000,$0892,$089A,$0000,$08A2,$08AA,$08B2,$08BA,$08C2,$08CA,$08D2,$0000,$0000,$0000,$0000,$0000
    DATA $0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
    DATA $0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
    DATA $0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
    DATA $0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
    DATA $0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
    DATA $0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
    DATA $0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
    DATA $0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
