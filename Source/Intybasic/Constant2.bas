
' =========================================================================
' Define Constants
' =========================================================================
CONST TITLE_ROW        = 6      ' Background row in which to print TITLE string
CONST TITLE_LENGTH     = 13     ' Length of TITLE string, in characters
CONST TITLE_COLOR      = CS_YELLOW  ' Color of TITLE string


CONST AUTHOR_ROW       = 10     ' Background row in which to print AUTHOR string
CONST AUTHOR_LENGTH    = 6      ' Length of AUTHOR string, in characters
CONST AUTHOR_COLOR     = CS_WHITE   ' Color of AUTHOR string (and copyright year)

CONST COLOR_BAR_ROW    = 5      ' Background row in which to display colored bars
CONST BRAND_ROW        = 3      ' Row in which to print the SDK brand and logo
CONST BRAND_COLOR      = CS_WHITE   ' Color of SDK brand and logo

CONST CARD_WIDTH       = 8      ' Width of a background card, in pixels
CONST CARD_HEIGHT      = 8      ' Height of a background card, in pixels

CONST DEBOUNCE_DELAY   = 2      ' Number of cycles to detect button press
CONST NO_KEY           = 0      ' Represents no user input
 
DEF FN TextCenterPos(aLength, aRow)  = SCREENPOS((((BACKGROUND_COLUMNS - aLength) + 1) / 2), aRow)
DEF FN SpritePosX(aColumn, anOffset) = ((aColumn + 1) * CARD_WIDTH ) + anOffset
DEF FN SpritePosY(aRow, anOffset)    = ((aRow    + 1) * CARD_HEIGHT) + anOffset

   ' Set Screen Mode to "Color Stack" and define the stack
    MODE   SCREEN_COLOR_STACK, STACK_BLACK, STACK_BROWN, STACK_BLACK, STACK_BROWN
    BORDER BORDER_BLACK
    rem DEFINE DEF00,5,Graphics
    CLS
    DEFINE 0,16,screen_bitmaps_0
    WAIT
    DEFINE 16,16,screen_bitmaps_1
   
    WAIT
    SCREEN screen_cards

      ' Print classic colored bars
    '  - Vertical bars on left
    PRINT AT SCREENPOS( 2, COLOR_BAR_ROW) COLOR CS_WHITE,     "\165"
    PRINT AT SCREENPOS( 4, COLOR_BAR_ROW) COLOR CS_YELLOW,    "\165"
    PRINT AT SCREENPOS( 6, COLOR_BAR_ROW) COLOR CS_GREEN,     "\165"
    PRINT AT SCREENPOS( 8, COLOR_BAR_ROW) COLOR CS_DARKGREEN, "\165"
    PRINT AT SCREENPOS( 9, COLOR_BAR_ROW) COLOR CS_RED, "II"

    '  - Vertical bars on right
    PRINT AT SCREENPOS(11, COLOR_BAR_ROW) COLOR CS_TAN,       "\164"
    PRINT AT SCREENPOS(13, COLOR_BAR_ROW) COLOR CS_RED,       "\164"
    PRINT AT SCREENPOS(15, COLOR_BAR_ROW) COLOR CS_BLUE,      "\164"
    PRINT AT SCREENPOS(17, COLOR_BAR_ROW) COLOR CS_WHITE,     "\164"


    PRINT AT TextCenterPos((AUTHOR_LENGTH + 4), AUTHOR_ROW) + 0, BG27 + AUTHOR_COLOR
    PRINT AT TextCenterPos((AUTHOR_LENGTH + 4), AUTHOR_ROW) + 1 COLOR AUTHOR_COLOR,  "2024 AOtta"
