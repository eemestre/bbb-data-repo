#include "libpruio/pruio.h"
#include "stdio.h"
#include "time.h"
#include "unistd.h"

int main(int argc, char **argv) {
  const uint32 tSamp = 123401;
  const uint32 tmr = 20000;
  const uint32 NoStep = 3;
  const uint32 NoFile = 2;
  const char *NamFil = "output.%u";
  struct timespec mSec;
  mSec.tv_nsec = 1000000;
  pruIo *io = pruio_new(PRUIO_DEF_ACTIVE, 0, 0, 0);
  if (io->Errr) {
    printf("constructor failed (%s)\n", io->Errr);
    return 1;
  }

  do {
    if (pruio_adc_setStep(io, 9, 0, 0, 0, 0)) {  //          step 9, AIN-0
      printf("step 9 configuration failed: (%s)\n", io->Errr);
      break;
    }
    if (pruio_adc_setStep(io, 10, 1, 0, 0, 0)) {  //         step 10, AIN-1
      printf("step 10 configuration failed: (%s)\n", io->Errr);
      break;
    }
    if (pruio_adc_setStep(io, 11, 2, 0, 0, 0)) {  //         step 11, AIN-2
      printf("step 11 configuration failed: (%s)\n", io->Errr);
      break;
    }

    uint32 mask = 7 << 9;
    uint32 tInd = tSamp * NoStep;
    uint32 half = ((io->ESize >> 2) / NoStep) * NoStep;

    if (half > tInd) {
      half = tInd;
    }  //       adapt size for small files
    uint32 samp = (half << 1) / NoStep;

    if (pruio_config(io, samp, mask, tmr, 0)) {  //       configure driver
      printf("config failed (%s)\n", io->Errr);
      break;
    }

    if (pruio_rb_start(io)) {
      printf("rb_start failed (%s)\n", io->Errr);
      break;
    }

    uint16 *p0 = io->Adc->Value;
    uint16 *p1 = p0 + half;
    uint32 n;
    char fName[20];
    for (n = 0; n < NoFile; n++) {
      sprintf(fName, NamFil, n);
      printf("Creating file %s\n", fName);
      FILE *oFile = fopen(fName, "wb");
      uint32 i = 0;
      while (i < tInd) {
        i += half;
        if (i > tInd) {  // fetch the rest(maybe no complete chunk)
          uint32 rest = tInd + half - i;
          uint32 iEnd = p1 >= p0 ? rest : rest + half;
          while (io->DRam[0] < iEnd) nanosleep(&mSec, NULL);
          printf("  writing samples %u-%u\n", tInd - rest, tInd - 1);
          fwrite(p0, sizeof(uint16), rest, oFile);
          uint16 *swap = p0;
          p0 = p1;
          p1 = swap;
          break;
        }
        if (p1 > p0)
          while (io->DRam[0] < half) nanosleep(&mSec, NULL);
        else
          while (io->DRam[0] > half) nanosleep(&mSec, NULL);
        printf("  writing samples %u-%u\n", i - half, i - 1);
        fwrite(p0, sizeof(uint16), half, oFile);
        uint16 *swap = p0;
        p0 = p1;
        p1 = swap;
      }
      fclose(oFile);
      printf("Finished file %s\n", fName);
    }
  } while (0);
  pruio_destroy(io);
  return 0;
}
