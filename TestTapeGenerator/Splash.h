#ifndef SPLASH_H
#define SPLASH_H

void splashscreen()
{
    LCD_Helper lcdhelper;
    char stringbuffer[255];
    sprintf(stringbuffer, "  %s", TESTTAPEGENERATOR_SW_VERSION);
    lcdhelper.line[0] = "Test Tape Generator";
    lcdhelper.line[1] = stringbuffer;
    lcdhelper.line[2] = TAPELIST_VERSION;
    lcdhelper.Show();
    delay(2000);
}

#endif // SPLASH_H
