enum { NONE, LOAD_CHAR, SHOW_CHAR, BETWEEN_CHAR } scroll_state = NONE;
uint8_t* scroll_ptr = NULL;

void setScrollMessage(uint8_t* msg) {
  scroll_ptr = msg;
  scroll_state = LOAD_CHAR;
}

void scrollDataSink(uint8_t dev, MD_MAX72XX::transformType_t t, uint8_t col) {}

uint8_t scrollDataSource(uint8_t dev, MD_MAX72XX::transformType_t t) {
  static uint8_t  curLen, showLen;
  static uint8_t  cBuf[15];
  uint8_t colData = 0;    // blank column is the default

  if (!scroll_ptr) {
    scroll_state = NONE;
    return(0);
  }

  switch(scroll_state) {
    case NONE:
      return(0);
    case LOAD_CHAR: // Load the next character from the font table

      if (*scroll_ptr == '\0') {
        // end of message
        scroll_ptr = NULL;
        scroll_state = NONE;
        return(0);
      }

      showLen = mx.getChar(*scroll_ptr++, sizeof(cBuf)/sizeof(cBuf[0]), cBuf);
      curLen = 0;
      scroll_state = SHOW_CHAR;

      // !! deliberately fall through to next state to start displaying

    case SHOW_CHAR: // display the next part of the character
      colData = cBuf[curLen++];
      if (curLen == showLen) {
        showLen = CHAR_SPACING;
        curLen = 0;
        scroll_state = BETWEEN_CHAR;
      }
      break;

    case BETWEEN_CHAR: // display inter-character spacing (blank columns)
      colData = 0;
      curLen++;
      if (curLen == showLen)
        scroll_state = LOAD_CHAR;
      break;

    default:
      scroll_state = LOAD_CHAR;
  }

  return(colData);
}

 void scrollText(void)
{
  static uint32_t prevTime = 0;

  // Is it time to scroll the text?
  if (millis() - prevTime >= SCROLL_DELAY)
  {
    mx.transform(MD_MAX72XX::TSL);  // scroll along - the callback will load all the data
    prevTime = millis();      // starting point for next time
  }
}


void printText(uint8_t modStart, uint8_t modEnd, char *pMsg)
// Print the text string to the LED matrix modules specified.
// Message area is padded with blank columns after printing.
{
  uint8_t   state = 0;
  uint8_t   curLen;
  uint16_t  showLen;
  uint8_t   cBuf[8];
  int16_t   col = ((modEnd + 1) * COL_SIZE) - 1;

  mx.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  do     // finite state machine to print the characters in the space available
  {
    switch(state)
    {
      case 0: // Load the next character from the font table
        // if we reached end of message, reset the message pointer
        if (*pMsg == '\0')
        {
          showLen = col - (modEnd * COL_SIZE);  // padding characters
          state = 2;
          break;
        }

        // retrieve the next character form the font file
        showLen = mx.getChar(*pMsg++, sizeof(cBuf)/sizeof(cBuf[0]), cBuf);
        curLen = 0;
        state++;
        // !! deliberately fall through to next state to start displaying

      case 1: // display the next part of the character
        mx.setColumn(col--, cBuf[curLen++]);

        // done with font character, now display the space between chars
        if (curLen == showLen)
        {
          showLen = CHAR_SPACING;
          state = 2;
        }
        break;

      case 2: // initialize state for displaying empty columns
        curLen = 0;
        state++;
        // fall through

      case 3:  // display inter-character spacing or end of message padding (blank columns)
        mx.setColumn(col--, 0);
        curLen++;
        if (curLen == showLen)
          state = 0;
        break;

      default:
        col = -1;   // this definitely ends the do loop
    }
  } while (col >= (modStart * COL_SIZE));

  mx.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}
