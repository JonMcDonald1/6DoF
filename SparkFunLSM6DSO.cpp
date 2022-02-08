
#include "SparkFunLSM6DSO.h"

LSM6DSOCore::LSM6DSOCore() { }

status_t LSM6DSOCore::beginCore(uint8_t deviceAddress, TwoWire &i2cPort)
{


  commInterface = I2C_MODE; 
  _i2cPort = &i2cPort;
  I2CAddress = deviceAddress;

	uint8_t partID;
	status_t returnError = readRegister(&partID, WHO_AM_I_REG);
	if( partID != 0x6C && returnError != IMU_SUCCESS)
		return returnError;
  else
    return IMU_SUCCESS;

}



//****************************************************************************//
//
//  ReadRegisterRegion
//
//  Parameters:
//    *outputPointer -- Pass &variable (base address of) to save read data to
//    address -- register to read
//    numBytes -- number of bytes to read
//
//  Note:  Does not know if the target memory space is an array or not, or
//    if there is the array is big enough.  if the variable passed is only
//    two bytes long and 3 bytes are requested, this will over-write some
//    other memory!
//
//****************************************************************************//
status_t LSM6DSOCore::readMultipleRegisters(uint8_t outputPointer[], uint8_t address, uint8_t numBytes)
{

	status_t returnError;
  uint8_t byteReturn;

	switch( commInterface ){

    case I2C_MODE:

      _i2cPort->beginTransmission(I2CAddress);
      _i2cPort->write(address);
      if( _i2cPort->endTransmission(false) != 0 )
        return IMU_HW_ERROR;

      byteReturn = _i2cPort->requestFrom(static_cast<uint8_t>(I2CAddress), static_cast<uint8_t>(numBytes));

      if( byteReturn == 0 )
        return IMU_HW_ERROR;

      for(size_t i = 0; i < numBytes; i++){
         outputPointer[i] =  _i2cPort->read(); 
      }

      return IMU_SUCCESS;

    default:
      return IMU_GENERIC_ERROR;

  }
}

//****************************************************************************//
//  readRegister
//
//  Parameters:
//    *outputPointer -- Pass &variable (address of) to save read data to
//    address -- register to read
//****************************************************************************//
status_t LSM6DSOCore::readRegister(uint8_t* outputPointer, uint8_t address) {

	status_t returnError; 
  uint8_t byteReturn;

	switch (commInterface) {

	case I2C_MODE:
    
		_i2cPort->beginTransmission(I2CAddress);
		_i2cPort->write(address);
		if( _i2cPort->endTransmission() != 0 )
			return IMU_HW_ERROR;

		byteReturn  = _i2cPort->requestFrom(static_cast<uint8_t>(I2CAddress), static_cast<uint8_t>(1));
    
    if( byteReturn == 0 )
      return IMU_HW_ERROR;

    *outputPointer = _i2cPort->read(); // receive a byte as a proper uint8_t

    return IMU_SUCCESS;
	
  default:
    return IMU_GENERIC_ERROR;
  }

}

//****************************************************************************//
//  readRegisterInt16
//
//  Parameters:
//    *outputPointer -- Pass &variable (base address of) to save read data to
//    address -- register to read
//****************************************************************************//
status_t LSM6DSOCore::readRegisterInt16(int16_t* outputPointer, uint8_t address) 
{
	uint8_t myBuffer[2];
	status_t returnError = readMultipleRegisters(myBuffer, address, 2);  //Does memory transfer
	int16_t output = myBuffer[0] | static_cast<uint16_t>(myBuffer[1] << 8);
	
	*outputPointer = output;
	return returnError;
}

//****************************************************************************//
//  writeRegister
//
//  Parameters:
//    address -- register to write
//    dataToWrite -- 8 bit data to write to register
//****************************************************************************//
status_t LSM6DSOCore::writeRegister(uint8_t address, uint8_t dataToWrite) {

	status_t returnError;

	switch (commInterface) {

    case I2C_MODE:
      //Write the byte
      _i2cPort->beginTransmission(I2CAddress);
      _i2cPort->write(address);
      _i2cPort->write(dataToWrite);
      if( _i2cPort->endTransmission() != 0 )
        return IMU_HW_ERROR;
      break;

    default:
      break;

	}

	return IMU_SUCCESS;
}

//****************************************************************************//
//  writeMultipleRegisters
//
//  Parameters:
//    inputPointer -- array to be written to device
//    address -- register to write
//    numBytes -- number of bytes contained in the array
//****************************************************************************//
status_t LSM6DSOCore::writeMultipleRegisters(uint8_t inputPointer[], uint8_t address, uint8_t numBytes) {

	status_t returnError;

	switch( commInterface ){

    case I2C_MODE:

      _i2cPort->beginTransmission(I2CAddress);
      _i2cPort->write(address);

      for(size_t i = 0; i < numBytes; i++){
         _i2cPort->write(inputPointer[i]); 
      }

      if( _i2cPort->endTransmission() != 0 )
        return IMU_HW_ERROR;
      else
        return IMU_SUCCESS;

    default:
      return IMU_GENERIC_ERROR;

  }
}


status_t LSM6DSOCore::enableEmbeddedFunctions(bool enable)
{
  uint8_t tempVal; 
  readRegister(&tempVal, FUNC_CFG_ACCESS);
  
  tempVal &= 0x7F;

  if( enable )
    tempVal |= 0x80;  
  else
    tempVal |= 0x7F; 

	status_t returnError = writeRegister( FUNC_CFG_ACCESS, tempVal );
	return returnError;
}

//****************************************************************************//
//
//  Main user class -- wrapper for the core class + maths
//
//  Construct with same rules as the core ( uint8_t busType, uint8_t inputArg )
//
//****************************************************************************//
LSM6DSO::LSM6DSO() 
{
	//Construct with these default imuSettings

	imuSettings.gyroEnabled = true;       //Can be 0 or 1
	imuSettings.gyroRange = 500;          //Max deg/s.  Can be: 125, 250, 500, 1000, 2000
	imuSettings.gyroSampleRate = 416;     //Hz.  Can be: 13, 26, 52, 104, 208, 416, 833, 1666
	imuSettings.gyroBandWidth = 400;      //Hz.  Can be: 50, 100, 200, 400;
	imuSettings.gyroFifoEnabled = 1;      //Set to include gyro in FIFO
	imuSettings.gyroAccelDecimation = 1;  //Set to include gyro in FIFO

	imuSettings.accelEnabled = true;
	imuSettings.accelRange = 8;         //Max G force readable.  Can be: 2, 4, 8, 16
	imuSettings.accelSampleRate = 416;  //Hz.  Can be: 1.6 (16), 12.5 (125), 26, 52, 104, 208, 416, 833, 1660, 3330, 6660
	imuSettings.accelFifoEnabled = 1;   //Set to include accelerometer in the FIFO

  imuSettings.fifoEnabled = true;
	imuSettings.fifoThreshold = 3000;   //Can be 0 to 4096 (16 bit bytes)
	imuSettings.fifoSampleRate = 416; 
	imuSettings.fifoModeWord = 0;       //Default off

	allOnesCounter = 0;
	nonSuccessCounter = 0;

}

bool LSM6DSO::begin(uint8_t address, TwoWire &i2cPort){

  if( address != DEFAULT_ADDRESS && address != ALT_ADDRESS )
    return false;

  uint8_t regVal;
	status_t returnError = beginCore(address, i2cPort);
  if( returnError != IMU_SUCCESS )
    return false;
  else
    return true; 
  
}


bool LSM6DSO::initialize(uint8_t settings){

  setIncrement();

  if( settings == BASIC_SETTINGS ){
    setAccelRange(8);
    setAccelDataRate(416);
    setGyroRange(500);
    setGyroDataRate(416);
    setBlockDataUpdate(true);
  }
 

  return true;

}

status_t LSM6DSO::beginSettings() {

	uint8_t dataToWrite = 0;  //Temporary variable

	//Setup the accelerometer******************************
	dataToWrite = 0; //Start Fresh!
	if ( imuSettings.accelEnabled == 1) {
    //Range
		switch (imuSettings.accelRange) {
		case 2:
			dataToWrite |= FS_XL_2g;
			break;
		case 4:
			dataToWrite |= FS_XL_4g;
			break;
		case 8:
			dataToWrite |= FS_XL_8g;
			break;
		default:  //set default case to 16(max)
		case 16:
			dataToWrite |= FS_XL_16g;
			break;
		}
		// Accelerometer ODR
		switch (imuSettings.accelSampleRate) {
		case 16:
			dataToWrite |= ODR_XL_1_6Hz;
			break;
		case 125:
			dataToWrite |= ODR_XL_12_5Hz;
			break;
		case 26:
			dataToWrite |= ODR_XL_26Hz;
			break;
		case 52:
			dataToWrite |= ODR_XL_52Hz;
			break;
		default:  //Set default to 104
		case 104:
			dataToWrite |= ODR_XL_104Hz;
			break;
		case 208:
			dataToWrite |= ODR_XL_208Hz;
			break;
		case 416:
			dataToWrite |= ODR_XL_416Hz;
			break;
		case 833:
			dataToWrite |= ODR_XL_833Hz;
			break;
		case 1660:
			dataToWrite |= ODR_XL_1660Hz;
			break;
		case 3330:
			dataToWrite |= ODR_XL_3330Hz;
			break;
		case 6660:
			dataToWrite |= ODR_XL_6660Hz;
			break;
		}
	}

  // Write Accelerometer Settings....
	writeRegister(CTRL1_XL, dataToWrite);

	//Setup the gyroscope**********************************************
	dataToWrite = 0; // Clear variable

	if ( imuSettings.gyroEnabled == 1) {
		switch (imuSettings.gyroRange) {
		case 125:
			dataToWrite |=  FS_G_125dps;
			break;
		case 245:
			dataToWrite |=  FS_G_250dps;
			break;
		case 500:
			dataToWrite |=  FS_G_500dps;
			break;
		case 1000:
			dataToWrite |=  FS_G_1000dps;
			break;
		default:  //Default to full 2000DPS range
		case 2000:
			dataToWrite |=  FS_G_2000dps;
			break;
		}
		switch (imuSettings.gyroSampleRate) { 
		case 125:
			dataToWrite |= ODR_GYRO_12_5Hz;
			break;
		case 26:
			dataToWrite |= ODR_GYRO_26Hz;
			break;
		case 52:
			dataToWrite |= ODR_GYRO_52Hz;
			break;
		default:  //Set default to 104
		case 104:
			dataToWrite |= ODR_GYRO_104Hz;
			break;
		case 208:
			dataToWrite |= ODR_GYRO_208Hz;
			break;
		case 416:
			dataToWrite |= ODR_GYRO_416Hz;
			break;
		case 833:
			dataToWrite |= ODR_GYRO_833Hz;
			break;
		case 1660:
			dataToWrite |= ODR_GYRO_1660Hz;
			break;
		case 3330:
			dataToWrite |= ODR_GYRO_3330Hz;
			break;
		case 6660:
			dataToWrite |= ODR_GYRO_6660Hz;
			break;
		}
	}
	
  // Write the gyroscope imuSettings. 
	writeRegister(CTRL2_G, dataToWrite);

	return IMU_SUCCESS;
}

// Address: 0x1E , bit[2:0]: default value is: 0x00
// Checks if there is new accelerometer, gyro, or temperature data.
uint8_t LSM6DSO::listenDataReady(){

  uint8_t regVal;
  status_t returnError = readRegister(&regVal, STATUS_REG);
  
  if( returnError != IMU_SUCCESS )
    return IMU_GENERIC_ERROR;
  else
    return regVal; 
}

// Address:0x12 CTRL3_C , bit[6] default value is: 0x00
// This function sets the BDU (Block Data Update) bit. Use when not employing
// the FIFO buffer.
bool LSM6DSO::setBlockDataUpdate(bool enable){

  uint8_t regVal;
  status_t returnError = readRegister(&regVal, CTRL3_C); 
  if( returnError != IMU_SUCCESS )
    return false;
    
  regVal &= 0xBF;
  regVal |= BDU_BLOCK_UPDATE;   

  returnError = writeRegister(CTRL3_C, regVal);  			
  if( returnError != IMU_SUCCESS )
    return false;
  else 
    return true;


}



// Address:0x15 , bit[4]: default value is: 0x00
// Sets whether high performance mode is on for the acclerometer, by default it is ON.
bool LSM6DSO::setHighPerfAccel(bool enable){

  uint8_t regVal;
  status_t returnError = readRegister(&regVal, CTRL6_C);
  if( returnError != IMU_SUCCESS )
    return false; 

  if( enable )
    regVal |=  HIGH_PERF_ACC_ENABLE; 
  else
    regVal |=  HIGH_PERF_ACC_DISABLE; 

  returnError = writeRegister(CTRL6_C, regVal);
  if( returnError != IMU_SUCCESS )
    return false; 
  else
    return true;
}

// Address:0x16 , bit[7]: default value is: 0x00
// Sets whether high performance mode is on for the gyroscope, by default it is ON.
bool LSM6DSO::setHighPerfGyro(bool enable){

  uint8_t regVal;
  status_t returnError = readRegister(&regVal, CTRL7_G);
  if( returnError != IMU_SUCCESS )
    return false; 

  if( enable )
    regVal |=  HIGH_PERF_GYRO_ENABLE; 
  else
    regVal |=  HIGH_PERF_GYRO_DISABLE; 

  returnError = writeRegister(CTRL7_G, regVal);
  if( returnError != IMU_SUCCESS )
    return false; 
  else
    return true;
}

//****************************************************************************//
//
//  Accelerometer section
//
//****************************************************************************//

// Address: 0x10 , bit[4:3]: default value is: 0x00 (2g) 
// Sets the acceleration range of the accleromter portion of the IMU.
bool LSM6DSO::setAccelRange(uint8_t range) {

  if( range < 0  | range > 16)
    return false; 

  uint8_t regVal;
  uint8_t fullScale; 
  status_t returnError = readRegister(&regVal, CTRL1_XL);
  if( returnError != IMU_SUCCESS )
      return false;

  fullScale = getAccelFullScale();

  // Can't have 16g with XL_FS_MODE == 1
  if( fullScale == 1 && range == 16 )
    range = 8;

  regVal &= FS_XL_MASK;

  switch( range ) {
    case 2:
      regVal |= FS_XL_2g;
      break;
    case 4:
      regVal |= FS_XL_4g;
      break;
    case 8:
      regVal |= FS_XL_8g;
      break;
    case 16:
      regVal |= FS_XL_16g;
      break;
    default:
      break;
  }

  returnError = writeRegister(CTRL1_XL, regVal);
  if( returnError != IMU_SUCCESS )
      return false;
  else
      return true;
}

// Address: 0x10 , bit[4:3]: default value is: 0x00 (2g) 
// Gets the acceleration range of the accleromter portion of the IMU.
// The value is dependent on the full scale bit (see getAccelFullScale).
uint8_t LSM6DSO::getAccelRange(){

  uint8_t regVal;
  uint8_t fullScale;

  status_t returnError = readRegister(&regVal, CTRL1_XL);
  if( returnError != IMU_SUCCESS )
    return IMU_GENERIC_ERROR;

  fullScale = getAccelFullScale();  
  regVal = (regVal & 0x0C) >> 2; 

  if( fullScale == 1 ){
    switch( regVal ){
      case 0: 
        return 2;
      case 1:
        return 2;
      case 2:
        return 4;
      case 3:
        return 8;
      default:
        return IMU_GENERIC_ERROR;
      }
    }
  else if( fullScale == 0 ){
    switch( regVal ){
      case 0: 
        return 2;
      case 1:
        return 16;
      case 2:
        return 4;
      case 3:
        return 8;
      default:
        return IMU_GENERIC_ERROR;
      }
  }
  else
    return IMU_GENERIC_ERROR;

}

// Address: 0x10, bit[7:4]: default value is: 0x00 (Power Down)
// Sets the output data rate of the accelerometer there-by enabling it. 
bool LSM6DSO::setAccelDataRate(uint16_t rate) {

  if( rate < 16  | rate > 6660) 
    return false; 

  uint8_t regVal;
  uint8_t highPerf;
  status_t returnError = readRegister(&regVal, CTRL1_XL);
  if( returnError != IMU_SUCCESS )
      return false;

  highPerf = getAccelHighPerf();

  // Can't have 1.6Hz and have high performance mode enabled.
  if( highPerf == 0 && rate == 16 ) 
    rate = 125;

  regVal &= ODR_XL_MASK;

  switch ( rate ) {
    case 0:
      regVal |= ODR_XL_DISABLE;
      break;
    case 16:
      regVal |= ODR_XL_1_6Hz;
      break;
    case 125:
      regVal |= ODR_XL_12_5Hz;
      break;
    case 26:
      regVal |= ODR_XL_26Hz;
      break;
    case 52:
      regVal |= ODR_XL_52Hz;
      break;
    case 104:
      regVal |= ODR_XL_104Hz;
      break;
    case 208:
      regVal |= ODR_XL_208Hz;
      break;
    case 416:
      regVal |= ODR_XL_416Hz;
      break;
    case 833:
      regVal |= ODR_XL_833Hz;
      break;
    case 1660:
      regVal |= ODR_XL_1660Hz;
      break;
    case 3330:
      regVal |= ODR_XL_3330Hz;
      break;
    case 6660:
      regVal |= ODR_XL_6660Hz;
      break;
    default:
      break;
  }

  returnError = writeRegister(CTRL1_XL, regVal);
  if( returnError != IMU_SUCCESS )
      return false;
  else
      return true;
}

// Address: 0x10, bit[7:4]: default value is: 0x00 (Power Down)
// Gets the output data rate of the accelerometer checking if high performance
// mode is enabled in which case the lowest possible data rate is 12.5Hz.
float LSM6DSO::getAccelDataRate(){

  uint8_t regVal;
  uint8_t highPerf;

  status_t returnError = readRegister(&regVal, CTRL1_XL);
  highPerf = getAccelHighPerf();

  if( returnError != IMU_SUCCESS )
    return static_cast<float>( IMU_GENERIC_ERROR );

   regVal &= ~ODR_XL_MASK; 

   switch( regVal ){ 
     case 0:
       return ODR_XL_DISABLE;
     case ODR_XL_1_6Hz: // Can't have 1.6 and high performance mode
       if( highPerf == 0 )
         return 12.5;
       return 1.6;
     case ODR_XL_12_5Hz:
       return 12.5;
     case ODR_XL_26Hz:
       return 26.0;
     case ODR_XL_52Hz:
       return 52.0;
     case ODR_XL_104Hz:
       return 104.0;
     case ODR_XL_208Hz:
       return 208.0;
     case ODR_XL_416Hz:
       return 416.0;
     case ODR_XL_833Hz:
       return 833.0;
     case ODR_XL_1660Hz:
       return 1660.0;
     case ODR_XL_3330Hz:
       return 3330.0;
     case ODR_XL_6660Hz:
       return 6660.0;
      default:
        return static_cast<float>(IMU_GENERIC_ERROR);
   }

}

// Address: 0x15, bit[4]: default value is: 0x00 (Enabled)
// Checks wheter high performance is enabled or disabled. 
uint8_t LSM6DSO::getAccelHighPerf(){

  uint8_t regVal;
  status_t returnError = readRegister(&regVal, CTRL6_C);

  if( returnError != IMU_SUCCESS )
    return IMU_GENERIC_ERROR;
  else
    return ((regVal & 0x10) >> 4); 

}

// Address: 0x17, bit[2]: default value is: 0x00 
// Checks whether the acclerometer is using "old" full scale or "new", see
// datasheet for more information.
uint8_t LSM6DSO::getAccelFullScale(){

  uint8_t regVal;
  status_t returnError = readRegister(&regVal, CTRL8_XL);

  if( returnError != IMU_SUCCESS )
    return IMU_GENERIC_ERROR;
  else
    return ((regVal & 0x02) >> 1); 
}

int16_t LSM6DSO::readRawAccelX() {

	int16_t output;
	status_t errorLevel = readRegisterInt16( &output, OUTX_L_A );
	if( errorLevel != IMU_SUCCESS )
	{
		if( errorLevel == IMU_ALL_ONES_WARNING )
			allOnesCounter++;
		else
			nonSuccessCounter++;
	}
	return output;
}

float LSM6DSO::readFloatAccelX() {
	float output = calcAccel(readRawAccelX());
	return output;
}

int16_t LSM6DSO::readRawAccelY()
{
	int16_t output;
	status_t errorLevel = readRegisterInt16( &output, OUTY_L_A );
	if( errorLevel != IMU_SUCCESS )
	{
		if( errorLevel == IMU_ALL_ONES_WARNING )
			allOnesCounter++;
		else
			nonSuccessCounter++;
	}
	return output;
}

float LSM6DSO::readFloatAccelY()
{
	float output = calcAccel(readRawAccelY());
	return output;
}

int16_t LSM6DSO::readRawAccelZ()
{
	int16_t output;
	status_t errorLevel = readRegisterInt16( &output, OUTZ_L_A );
	if( errorLevel != IMU_SUCCESS )
	{
		if( errorLevel == IMU_ALL_ONES_WARNING )
			allOnesCounter++;
		else
			nonSuccessCounter++;
	}
	return output;
}

float LSM6DSO::readFloatAccelZ()
{
	float output = calcAccel(readRawAccelZ());
	return output;
}

float LSM6DSO::calcAccel( int16_t input )
{
  uint8_t accelRange; 
  uint8_t scale;
  float output;

  readRegister(&accelRange, CTRL1_XL);
  scale = (accelRange >> 1) & 0x01;
  accelRange = (accelRange >> 2) & (0x03);  
  
  if( scale == 0 ) {
    switch( accelRange ){
      case 0:// Register value 0: 2g
        output = (static_cast<float>(input) * (.061)) / 1000;
        break;
      case 1: //Register value 1 : 16g
        output = (static_cast<float>(input) * (.488)) / 1000;
        break;
      case 2: //Register value 2 : 4g
        output = (static_cast<float>(input) * (.122)) / 1000;
        break;
      case 3://Register value 3: 8g
        output = (static_cast<float>(input) * (.244)) / 1000;
        break;
    }
  }

  if( scale == 1 ){
    switch( accelRange ){
      case 0: //Register value 0: 2g
        output = (static_cast<float>(input) * (0.061)) / 1000;
        break;
      case 1://Register value 1: 2g
        output = (static_cast<float>(input) * (0.061)) / 1000;
        break;
      case 2://Register value 2: 4g
        output = (static_cast<float>(input) * (.122)) / 1000;
        break;
      case 3://Register value 3: 8g
        output = (static_cast<float>(input) * (.244)) / 1000;
        break;
    }
  }

	return output;
}

//****************************************************************************//
//
//  Gyroscope section
//
//****************************************************************************//

// Address:CTRL2_G , bit[7:4]: default value is: 0x00.
// Sets the gyro's output data rate thereby enabling it.  
bool LSM6DSO::setGyroDataRate(uint16_t rate) {

  if( rate < 0 | rate > 6660 ) 
    return false; 

  uint8_t regVal;
  status_t returnError = readRegister(&regVal, CTRL2_G);
  if( returnError != IMU_SUCCESS )
      return false;

  regVal &= ODR_GYRO_MASK;

  switch( rate ) {
    case 0:
      regVal |= ODR_GYRO_DISABLE;
      break;
    case 125:
      regVal |= ODR_GYRO_12_5Hz;
      break;
    case 26:
      regVal |= ODR_GYRO_26Hz;
      break;
    case 52:
      regVal |= ODR_GYRO_52Hz;
      break;
    case 104:
      regVal |= ODR_GYRO_104Hz;
      break;
    case 208:
      regVal |= ODR_GYRO_208Hz;
      break;
    case 416:
      regVal |= ODR_GYRO_416Hz;
      break;
    case 833:
      regVal |= ODR_GYRO_833Hz;
      break;
    case 1660:
      regVal |= ODR_GYRO_1660Hz;
      break;
    case 3330:
      regVal |= ODR_GYRO_3330Hz;
      break;
    case 6660:
      regVal |= ODR_GYRO_6660Hz;
      break;
    default:
      break;
  }

  returnError = writeRegister(CTRL2_G, regVal);
  if( returnError != IMU_SUCCESS )
      return false;
  else
      return true;
}

// Address:CTRL2_G , bit[7:4]: default value is:0x00 
// Gets the gyro's data rate. A data rate of 0, implies that the gyro portion
// of the IMU is disabled. 
float LSM6DSO::getGyroDataRate(){

  uint8_t regVal;
  status_t returnError = readRegister(&regVal, CTRL2_G);

  if( returnError != IMU_SUCCESS )
    return static_cast<float>(IMU_GENERIC_ERROR);

  regVal &= ~ODR_GYRO_MASK;

  switch( regVal ){
    case ODR_GYRO_DISABLE:
      return 0.0;
    case ODR_GYRO_12_5Hz:
      return 12.5;
    case ODR_GYRO_26Hz:
      return 26.5;
    case ODR_GYRO_52Hz:
      return 52.0;
    case ODR_GYRO_104Hz:
      return 104.0;
    case ODR_GYRO_208Hz:
      return 208.0;
    case ODR_GYRO_416Hz:
      return 416.0;
    case ODR_GYRO_833Hz:
      return 833.0;
    case ODR_GYRO_1660Hz:
      return 1660.0;
    case ODR_GYRO_3330Hz:
      return 3330.0;
    case ODR_GYRO_6660Hz:
      return 6660.0;
    default:
      return static_cast<float>(IMU_GENERIC_ERROR);
  }

}

// Address: 0x11, bit[3:0]: default value is: 0x00
// Sets the gyroscope's range.
bool LSM6DSO::setGyroRange(uint16_t range) {

  if( range < 250 | range > 2000)
    return false;

  uint8_t regVal;
  status_t returnError = readRegister(&regVal, CTRL2_G);
  if( returnError != IMU_SUCCESS )
      return false;

  regVal &= FS_G_MASK;

  switch( range ){
    case 125:
      regVal |= FS_G_125dps;
      break;
    case 250:
      regVal |= FS_G_250dps;
      break;
    case 500:
      regVal |= FS_G_500dps;
      break;
    case 1000:
      regVal |= FS_G_1000dps;
      break;
    case 2000:
      regVal |= FS_G_2000dps;
      break;
  }

  returnError = writeRegister(CTRL2_G, regVal);
  if( returnError != IMU_SUCCESS )
      return false;
  else
      return true;
}

// Address: 0x11, bit[3:0]: default value is: 0x00
// Gets the gyroscope's range.
uint16_t LSM6DSO::getGyroRange(){

  uint8_t regVal;

  status_t returnError = readRegister(&regVal, CTRL2_G);
  if( returnError != IMU_SUCCESS )
    return IMU_GENERIC_ERROR;

  regVal &= ~FS_G_MASK;
  
  switch( regVal ){
    case FS_G_125dps:
      return 125;
    case FS_G_250dps:
      return 250;
    case FS_G_500dps:
      return 500;
    case FS_G_1000dps:
      return 1000;
    case FS_G_2000dps:
      return 2000;
    default:
      return IMU_GENERIC_ERROR;
  }
}

int16_t LSM6DSO::readRawGyroX() {

	int16_t output;
	status_t errorLevel = readRegisterInt16( &output, OUTX_L_G );

	if( errorLevel != IMU_SUCCESS ) {
		if( errorLevel == IMU_ALL_ONES_WARNING )
			allOnesCounter++;
		else
			nonSuccessCounter++;
	}

	return output;
}

float LSM6DSO::readFloatGyroX() {

	float output = calcGyro(readRawGyroX());
	return output;
}

int16_t LSM6DSO::readRawGyroY() {

	int16_t output;
	status_t errorLevel = readRegisterInt16( &output, OUTY_L_G );

	if( errorLevel != IMU_SUCCESS ) {
		if( errorLevel == IMU_ALL_ONES_WARNING )
			allOnesCounter++;
		else
			nonSuccessCounter++;
	}

	return output;
}

float LSM6DSO::readFloatGyroY() {
  
	float output = calcGyro(readRawGyroY());
	return output;
}

int16_t LSM6DSO::readRawGyroZ() {

	int16_t output;
	status_t errorLevel = readRegisterInt16( &output, OUTZ_L_G );

	if( errorLevel != IMU_SUCCESS ) {
		if( errorLevel == IMU_ALL_ONES_WARNING )
			allOnesCounter++;
		else
			nonSuccessCounter++;
	}

	return output;
}

float LSM6DSO::readFloatGyroZ() {

	float output = calcGyro(readRawGyroZ());
	return output;

}

float LSM6DSO::calcGyro( int16_t input ) {

	uint8_t gyroRange;  
  uint8_t fullScale;
  float output; 

  readRegister(&gyroRange, CTRL2_G) ;
  fullScale = (gyroRange >> 1) & 0x01; 
  gyroRange = (gyroRange >> 2) & 0x03; 

  if( fullScale )
    output = (static_cast<float>(input) * 4.375)/1000;
  else {
    switch( gyroRange ){
      case 0:
        output = (static_cast<float>(input) * 8.75)/1000;
        break;
      case 1:
        output = (static_cast<float>(input) * 17.50)/1000;
        break;
      case 2:
        output = (static_cast<float>(input) * 35)/1000;
        break;
      case 3:
        output = (static_cast<float>(input) * 70)/1000;
        break;
    }
  }

  return output;
}

//****************************************************************************//
//
//  Temperature section
//
//****************************************************************************//
// Removed
//****************************************************************************//
//
//  FIFO section
//
//****************************************************************************//
//Removed 

// DO NOT TOUCH THE FOLLOWING FUNCTIONS BELOW , in initialize()

// Used In initialize function
// Address: 0x12 , bit[4]: default value is: 0x01
// Sets register iteration when making multiple reads.
bool LSM6DSO::setIncrement(bool enable) {

  uint8_t regVal;
  status_t returnError = readRegister(&regVal, CTRL3_C);
  if( returnError != IMU_SUCCESS )
      return false;

  regVal &= 0xFD;
  regVal |= IF_INC_ENABLED;

  returnError = writeRegister(CTRL3_C, regVal);
  if( returnError != IMU_SUCCESS )
      return false;
  else
      return true;
}

// DO NOT TOUCH THE FOLLOWING FUNCTIONS ABOVE

