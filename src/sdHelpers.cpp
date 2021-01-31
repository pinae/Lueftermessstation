#include <Arduino.h>
#include <sdios.h>
#include <SdFat.h>

extern ArduinoOutStream cout;

void printSpiPins() {
    cout << F("\nSPI pins:\n");
    cout << F("MISO: ") << int(MISO) << endl;
    cout << F("MOSI: ") << int(MOSI) << endl;
    cout << F("SCK:  ") << int(SCK) << endl;
    cout << F("SS:   ") << int(SS) << endl;
}

void cardOrSpeed() {
    cout << F("Try another SD card or reduce the SPI bus speed.\n");
    cout << F("Edit SPI_SPEED in this program to change it.\n");
}

void reformatMsg() {
    cout << F("Try reformatting the card.  For best results use\n");
    cout << F("the SdFormatter program in SdFat/examples or download\n");
    cout << F("and use SDFormatter from www.sdcard.org/downloads.\n");
}

void printSdInitError(int errorCode, int errorData) {
    cout << F(
             "\nSD initialization failed.\n"
             "Do not reformat the card!\n"
             "Is the card correctly inserted?\n"
             "Is chipSelect set to the correct value?\n"
             "Does another SPI device need to be disabled?\n"
             "Is there a wiring/soldering problem?\n");
    cout << F("\nerrorCode: ") << hex << showbase;
    cout << errorCode;
    cout << F(", errorData: ") << errorData;
    cout << dec << noshowbase << endl;
}

void printCardSizeInfo(SdFat sd) {
    uint32_t size = sd.card()->sectorCount();
    if (size == 0) {
        cout << F("Can't determine the card size.\n");
        cardOrSpeed();
        return;
    }
    uint32_t sizeMB = 0.000512 * size + 0.5;
    cout << F("Card size: ") << sizeMB;
    cout << F(" MB (MB = 1,000,000 bytes)") << endl;
    cout << F("Volume is FAT") << int(sd.vol()->fatType());
    cout << F(", Cluster size (bytes): ") << 512L * sd.vol()->sectorsPerCluster();
    cout << endl << endl;

    cout << F("Files found (date time size name):\n");
    sd.ls(LS_R | LS_DATE | LS_SIZE);

    if ((sizeMB > 1100 && sd.vol()->sectorsPerCluster() < 64)
        || (sizeMB < 2200 && sd.vol()->fatType() == 32)) {
        cout << F("\nThis card should be reformatted for best performance.\n");
        cout << F("Use a cluster size of 32 KB for cards larger than 1 GB.\n");
        cout << F("Only cards larger than 2 GB should be formatted FAT32.\n");
        reformatMsg();
        return;
    }
}