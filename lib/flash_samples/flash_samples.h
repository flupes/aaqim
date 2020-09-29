#ifndef AAQIM_FLASH_SAMPLES_H
#define AAQIM_FLASH_SAMPLES_H

#include <stdint.h>
#include <stdlib.h>

/**
 * Save or read samples on flash.
 *
 * The class implements a ring buffer and recycle the oldes flash memory when
 * the capacity is reached.
 * The class takes care of erasing the proper sector if it was already in use.
 *
 * Fully dependant on the specific ESP8266 flash implementation (not a general
 * utility). Store samples on the flash memory normally reserved for the File
 * System, and allow easy access to the indexed samples.
 *
 */
class FlashSamples {
 public:
  /** Declare the flash accessor.
   *
   * @param sampleSize Size of samples to be stored in bytes
   * @param sampleLength Desired total number of samples to store on flash
   * The sampleLength will be clamped to the maximum flash capacity.
   * @param startOffset Optional offset from the nominal flash storage (bytes)
   * By default, startOffset is zero, and then the samples will be start to be
   * stored at the begining of the ESP8266 filesystem flash area.
   */

  FlashSamples(size_t sampleSize, size_t samplesLength,
               uint32_t startOffset = 0);

  /** Retrieve the first/last sample addresses on the existing storage.
   *
   * This is not part of the constructor, to allow the user to only start
   * accessing the flash when desired.
   * @param erase Default false. If set to true, the flash used by this
   * class is first erase. This is a debug scenario only.
   */
  void Begin(bool erase = false);

  /** Returns the number of sample currently stored on flash.
   *
   * @return current number of samples, or UINT32_MAX if flash is not scanned
   * yet.
   */
  size_t NumberOfSamples();

  /** Write the given sample data to flash
   * @param pointer to the data to store permanently
   * @return true if the sample was successfully writen, false otherwise
   */
  bool StoreSample(uint32_t* data);

  /** Retrieve the sample with the given index
   * @param index Index of the desired sample. 0 is the most recent sample (end
   *              of the serie) and NumberOfSample would be the oldest sample
   *              in the serie.
   * @param data Where to store the data read. The caller is responsible to have
   *             properly allocated this memory space
   * @return true if a sample at the given index could be read, false otherwise
   *         (like index out of range)
   */
  bool ReadSample(size_t index, uint32_t* data);

  /** Returns the number of sectors used by the FlashSample storage
   */
  size_t SectorsInUse();

  /** Returns the actual FlashSample capacity in number of samples.
   * 
   * It may differ from the requested size: it will be at least two sectors of samples,
   * and since one sector needs to be cleared for rewrite, it will be one sector less
   * than the size requested in samples.
   */
  size_t NominalCapacity();

  void Info();

 private:
  uint32_t sampleSize_;         /** Size of samples to store (fixed size for all
                                   storage) */
  uint32_t flashStorageStart_;  /** Start of the flash to use (expressed
                                  using the full flash range, typically start
                                  is around 3M) */
  uint32_t flashStorageEnd_;    /** Last address of flash to use */
  uint32_t flashStorageLength_; /** Size of the flash storage to be used */
  uint32_t flashSectorSize_;    /** Flash sector size */
  uint32_t firstSampleAddr_;    /** Address of the firt (=older) sample (not
                                   necessary   zero since this is a ring buffer */
  uint32_t lastSampleAddr_;     /** Address of the last sample recoded */
  bool scanned_; /** Was Begin() called once to scane the flash? */
  bool empty_;   /** Is the flash area empty */
};

#endif
