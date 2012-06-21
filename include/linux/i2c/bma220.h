/*
 * This software program is licensed subject to the GNU General Public License
 * (GPL).Version 2,June 1991, available at http://www.fsf.org/copyleft/gpl.html

 * (C) Copyright 2010 Bosch Sensortec GmbH
 * All Rights Reserved
 */



#ifndef __BMA220_H__
#define __BMA220_H__

#define BMA220_MAJOR	101
#define BMA220_MINOR	0

#define BMA220_IOC_MAGIC 'B'

#define BMA220_SET_SLEEP_EN					_IOWR(BMA220_IOC_MAGIC,0, unsigned char)
#define BMA220_SET_SUSPEND					_IO(BMA220_IOC_MAGIC,1)
#define BMA220_READ_ACCEL_X					_IOWR(BMA220_IOC_MAGIC,2, signed char)
#define BMA220_READ_ACCEL_Y					_IOWR(BMA220_IOC_MAGIC,3, signed char)
#define BMA220_READ_ACCEL_Z					_IOWR(BMA220_IOC_MAGIC,4, signed char)
#define BMA220_SET_MODE						_IOWR(BMA220_IOC_MAGIC,5, unsigned char)
#define BMA220_GET_MODE						_IOWR(BMA220_IOC_MAGIC,6, unsigned char)
#define BMA220_SET_RANGE						_IOWR(BMA220_IOC_MAGIC,7, unsigned char)
#define BMA220_GET_RANGE						_IOWR(BMA220_IOC_MAGIC,8, unsigned char)
#define BMA220_SET_BANDWIDTH					_IOWR(BMA220_IOC_MAGIC,9, unsigned char)
#define BMA220_GET_BANDWIDTH					_IOWR(BMA220_IOC_MAGIC,10,unsigned char)
#define BMA220_SET_SC_FILT_CONFIG				_IOWR(BMA220_IOC_MAGIC,11,unsigned char)
#define BMA220_RESET_INTERRUPT					_IO(BMA220_IOC_MAGIC,12)
#define BMA220_GET_DIRECTION_STATUS_REGISTER	_IOWR(BMA220_IOC_MAGIC,13,unsigned char)
#define BMA220_GET_INTERRUPT_STATUS_REGISTER	_IOWR(BMA220_IOC_MAGIC,14,unsigned char)
#define BMA220_SOFT_RESET						_IO(BMA220_IOC_MAGIC,15)
#define BMA220_SET_LATCH_INT					_IOWR(BMA220_IOC_MAGIC,16,unsigned char)
#define BMA220_SET_EN_HIGH_XYZ				_IOWR(BMA220_IOC_MAGIC,17,unsigned char)
#define BMA220_SET_HIGH_TH					_IOWR(BMA220_IOC_MAGIC,18,unsigned char)
#define BMA220_SET_HIGH_HY					_IOWR(BMA220_IOC_MAGIC,19,unsigned char)
#define BMA220_SET_HIGH_DUR					_IOWR(BMA220_IOC_MAGIC,20,unsigned char)
#define BMA220_SET_EN_LOW						_IOWR(BMA220_IOC_MAGIC,21,unsigned char)
#define BMA220_SET_LOW_TH						_IOWR(BMA220_IOC_MAGIC,22,unsigned char)
#define BMA220_SET_LOW_HY						_IOWR(BMA220_IOC_MAGIC,23,unsigned char)
#define BMA220_SET_LOW_DUR					_IOWR(BMA220_IOC_MAGIC,24,unsigned char)
#define BMA220_SET_SERIAL_HIGH_BW				_IOWR(BMA220_IOC_MAGIC,25,unsigned char)
#define BMA220_READ_ACCEL_XYZ					_IOWR(BMA220_IOC_MAGIC,26,signed char)
#define BMA220_SET_EN_ORIENT					_IOWR(BMA220_IOC_MAGIC,27,unsigned char)
#define BMA220_SET_ORIENT_EX					_IOWR(BMA220_IOC_MAGIC,28,unsigned char)
#define BMA220_GET_ORIENTATION					_IOWR(BMA220_IOC_MAGIC,29,unsigned char)
#define BMA220_SET_EN_TT_XYZ					_IOWR(BMA220_IOC_MAGIC,30,unsigned char)
#define BMA220_SET_TT_TH						_IOWR(BMA220_IOC_MAGIC,31,unsigned char)
#define BMA220_SET_TT_DUR						_IOWR(BMA220_IOC_MAGIC,32,unsigned char)
#define BMA220_SET_TT_FILT						_IOWR(BMA220_IOC_MAGIC,33,unsigned char)
#define BMA220_SET_EN_SLOPE_XYZ				_IOWR(BMA220_IOC_MAGIC,34,unsigned char)
#define BMA220_SET_EN_DATA					_IOWR(BMA220_IOC_MAGIC,35,unsigned char)
#define BMA220_SET_SLOPE_TH					_IOWR(BMA220_IOC_MAGIC,36,unsigned char)
#define BMA220_SET_SLOPE_DUR					_IOWR(BMA220_IOC_MAGIC,37,unsigned char)
#define BMA220_SET_SLOPE_FILT					_IOWR(BMA220_IOC_MAGIC,38,unsigned char)
#define BMA220_SET_CAL_TRIGGER					_IOWR(BMA220_IOC_MAGIC,39,unsigned char)
#define BMA220_GET_CAL_RDY					_IOWR(BMA220_IOC_MAGIC,40,unsigned char)
#define BMA220_SET_HP_XYZ_EN					_IOWR(BMA220_IOC_MAGIC,41,unsigned char)
#define BMA220_SET_OFFSET_TARGET_X				_IOWR(BMA220_IOC_MAGIC,42,unsigned char)
#define BMA220_SET_OFFSET_TARGET_Y				_IOWR(BMA220_IOC_MAGIC,43,unsigned char)
#define BMA220_SET_OFFSET_TARGET_Z				_IOWR(BMA220_IOC_MAGIC,44,unsigned char)
#define BMA220_SET_SLEEP_DUR					_IOWR(BMA220_IOC_MAGIC,45,unsigned char)
#define BMA220_GET_SLEEP_DUR					_IOWR(BMA220_IOC_MAGIC,46,unsigned char)
#define BMA220_SET_OFFSET_RESET				_IOWR(BMA220_IOC_MAGIC,47,unsigned char)
#define BMA220_SET_CUT_OFF_SPEED				_IOWR(BMA220_IOC_MAGIC,48,unsigned char)
#define BMA220_SET_CAL_MANUAL   				_IOWR(BMA220_IOC_MAGIC,49,unsigned char)
#define BMA220_SET_SBIST						_IOWR(BMA220_IOC_MAGIC,50,unsigned char)
#define BMA220_SET_INTERRUPT_REGISTER			_IOWR(BMA220_IOC_MAGIC,51,unsigned char)
#define BMA220_SET_DIRECTION_INTERRUPT_REGISTER	_IOWR(BMA220_IOC_MAGIC,52,unsigned char)
#define BMA220_GET_ORIENT_INT					_IOWR(BMA220_IOC_MAGIC,53,unsigned char)
#define BMA220_SET_ORIENT_BLOCKING				_IOWR(BMA220_IOC_MAGIC,54,unsigned char)
#define BMA220_GET_CHIP_ID  					_IOWR(BMA220_IOC_MAGIC,55,unsigned char)
#define BMA220_GET_SC_FILT_CONFIG				_IOWR(BMA220_IOC_MAGIC,56,unsigned char)
#define BMA220_GET_SLEEP_EN					_IOWR(BMA220_IOC_MAGIC,57,unsigned char)
#define BMA220_GET_SERIAL_HIGH_BW				_IOWR(BMA220_IOC_MAGIC,58,unsigned char)
#define BMA220_GET_LATCH_INT					_IOWR(BMA220_IOC_MAGIC,59,unsigned char)
#define BMA220_GET_EN_DATA					_IOWR(BMA220_IOC_MAGIC,60,unsigned char)
#define BMA220_GET_EN_HIGH_XYZ				_IOWR(BMA220_IOC_MAGIC,61,unsigned char)
#define BMA220_GET_HIGH_TH					_IOWR(BMA220_IOC_MAGIC,62,unsigned char)
#define BMA220_GET_HIGH_HY					_IOWR(BMA220_IOC_MAGIC,63,unsigned char)
#define BMA220_GET_HIGH_DUR					_IOWR(BMA220_IOC_MAGIC,64,unsigned char)
#define BMA220_GET_EN_LOW						_IOWR(BMA220_IOC_MAGIC,65,unsigned char)
#define BMA220_GET_LOW_TH						_IOWR(BMA220_IOC_MAGIC,66,unsigned char)
#define BMA220_GET_LOW_HY						_IOWR(BMA220_IOC_MAGIC,67,unsigned char)
#define BMA220_GET_LOW_DUR					_IOWR(BMA220_IOC_MAGIC,68,unsigned char)
#define BMA220_GET_EN_ORIENT					_IOWR(BMA220_IOC_MAGIC,69,unsigned char)
#define BMA220_GET_ORIENT_EX					_IOWR(BMA220_IOC_MAGIC,70,unsigned char)
#define BMA220_GET_ORIENT_BLOCKING				_IOWR(BMA220_IOC_MAGIC,71,unsigned char)
#define BMA220_GET_EN_TT_XYZ					_IOWR(BMA220_IOC_MAGIC,72,unsigned char)
#define BMA220_GET_TT_TH						_IOWR(BMA220_IOC_MAGIC,73,unsigned char)
#define BMA220_GET_TT_DUR						_IOWR(BMA220_IOC_MAGIC,74,unsigned char)
#define BMA220_GET_TT_FILT						_IOWR(BMA220_IOC_MAGIC,75,unsigned char)
#define BMA220_SET_TT_SAMP			    		_IOWR(BMA220_IOC_MAGIC,76,unsigned char)
#define BMA220_GET_TT_SAMP			    		_IOWR(BMA220_IOC_MAGIC,77,unsigned char)
#define BMA220_SET_TIP_EN			    			_IOWR(BMA220_IOC_MAGIC,78,unsigned char)
#define BMA220_GET_TIP_EN			    			_IOWR(BMA220_IOC_MAGIC,79,unsigned char)
#define BMA220_GET_EN_SLOPE_XYZ				_IOWR(BMA220_IOC_MAGIC,80,unsigned char)
#define BMA220_GET_SLOPE_TH					_IOWR(BMA220_IOC_MAGIC,81,unsigned char)
#define BMA220_GET_SLOPE_DUR					_IOWR(BMA220_IOC_MAGIC,82,unsigned char)
#define BMA220_GET_SLOPE_FILT					_IOWR(BMA220_IOC_MAGIC,83,unsigned char)
#define BMA220_GET_HP_XYZ_EN					_IOWR(BMA220_IOC_MAGIC,84,unsigned char)
#define BMA220_GET_OFFSET_TARGET_X			_IOWR(BMA220_IOC_MAGIC,85,unsigned char)
#define BMA220_GET_OFFSET_TARGET_Y			_IOWR(BMA220_IOC_MAGIC,86,unsigned char)
#define BMA220_GET_OFFSET_TARGET_Z			_IOWR(BMA220_IOC_MAGIC,87,unsigned char)
#define BMA220_GET_CUT_OFF_SPEED				_IOWR(BMA220_IOC_MAGIC,88,unsigned char)
#define BMA220_GET_CAL_MANUAL   				_IOWR(BMA220_IOC_MAGIC,89,unsigned char)
#define BMA220_SET_OFFSET_XYZ					_IOWR(BMA220_IOC_MAGIC,90,signed char)
#define BMA220_GET_OFFSET_XYZ					_IOWR(BMA220_IOC_MAGIC,91,signed char)
#define BMA220_GET_SENSOR_TYPE				_IOR(BMA220_IOC_MAGIC,92,signed char)

#define BMA220_IOC_MAXNR			93


/* BMA220 Macro for read and write communication */


/**
   define for used read and write macros 
*/


/** Define the calling convention of YOUR bus communication routine.
	\note This includes types of parameters. This example shows the configuration for an SPI bus link.

	If your communication function looks like this:

	write_my_bus_xy(unsigned char device_addr, unsigned char register_addr, unsigned char * data, unsigned char length);

	The BMA220_WR_FUNC_PTR would equal:

    #define	BMA220_WR_FUNC_PTR char (* bus_write)(unsigned char, unsigned char, unsigned char *, unsigned char)
	
	Parameters can be mixed as needed refer to the \ref BMA220_BUS_WRITE_FUNC  macro.
	

*/
#define BMA220_WR_FUNC_PTR char (* bus_write)( unsigned char, unsigned char, unsigned char *, unsigned char)



/** link makro between API function calls and bus write function
	\note The bus write function can change since this is a system dependant issue.

	If the bus_write parameter calling order is like: reg_addr, reg_data, wr_len it would be as it is here.

	If the parameters are differently ordered or your communication function like I2C need to know the device address, 
	you can change this macro accordingly.


	define BMA220_BUS_WRITE_FUNC(dev_addr, reg_addr, reg_data, wr_len)\
           bus_write(dev_addr, reg_addr, reg_data, wr_len)

    This macro lets all API functions call YOUR communication routine in a way that equals your definition in the \ref BMA220_WR_FUNC_PTR definition.


	      
*/
#define BMA220_BUS_WRITE_FUNC(dev_addr, reg_addr, reg_data, wr_len)\
           bus_write( dev_addr, reg_addr, reg_data, wr_len)


/** Define the calling convention of YOUR bus communication routine.
	\note This includes types of parameters. This example shows the configuration for an SPI bus link.

	If your communication function looks like this:

	read_my_bus_xy(unsigned char device_addr, unsigned char register_addr, unsigned char * data, unsigned char length);

	The BMA220_RD_FUNC_PTR would equal:

    #define	BMA220_RD_FUNC_PTR char (* bus_read)(unsigned char, unsigned char, unsigned char *, unsigned char)
	
	Parameters can be mixed as needed refer to the \ref BMA220_BUS_READ_FUNC  macro.
	

*/

#define BMA220_SPI_RD_MASK 0x80   /* for spi read transactions on SPI the MSB has to be set */
#define BMA220_RD_FUNC_PTR char (* bus_read)( unsigned char,  unsigned char, unsigned char *, unsigned char)


/** link makro between API function calls and bus read function
	\note The bus write function can change since this is a system dependant issue.

	If the bus_read parameter calling order is like: reg_addr, reg_data, wr_len it would be as it is here.

	If the parameters are differently ordered or your communication function like I2C need to know the device address, 
	you can change this macro accordingly.


	define BMA220_BUS_READ_FUNC(dev_addr, reg_addr, reg_data, wr_len)\
           bus_read(dev_addr, reg_addr, reg_data, wr_len)

    This macro lets all API functions call YOUR communication routine in a way that equals your definition in the \ref BMA220_WR_FUNC_PTR definition.

	\note: this macro also includes the "MSB='1'" for reading BMA220 addresses. 
      
*/
#define BMA220_BUS_READ_FUNC(dev_addr, reg_addr, reg_data, r_len)\
           bus_read(dev_addr, reg_addr, reg_data, r_len)




#define BMA220_SOFTRESET_REG			0x19
#define BMA220_SUSPEND_MODE_REG			0x18
#define BMA220_SPI_MODE_SELECT_REG		0x17
#define BMA220_OFFSET_Z_RESULT_REG		0x16
#define BMA220_OFFSET_Y_RESULT_REG		0x15
#define BMA220_OFFSET_X_RESULT_REG		0x14
#define BMA220_OFFSET_TARGET_REG		0x13
#define BMA220_HPASS_CONFIG_REG			0x12
#define BMA220_RANGE_SELFTEST_REG		0x11
#define BMA220_BANDWIDTH_CONFIG_REG		0x10
#define BMA220_SLEEP_CONFIG_REG			0x0F
#define BMA220_INT_ENABLE2_REG			0x0E
#define BMA220_INT_ENABLE1_REG			0x0D
#define BMA220_INT_STATUS_REG			0x0C
#define BMA220_DIRECTION_STATUS_REG		0x0B
#define BMA220_DIRECTION_CONFIG_REG		0x0A
#define BMA220_SLOPE_DET_CONFIG_REG		0x09
#define BMA220_TAP_SENSING_CONFIG_REG	0x08
#define BMA220_LG_CONFIG_REG			0x07
#define BMA220_HG_LG_THRESHOLD_REG		0x06
#define BMA220_HG_CONFIG_REG			0x05
#define BMA220_Z_AXIS_DATA_REG			0x04
#define BMA220_Y_AXIS_DATA_REG			0x03
#define BMA220_X_AXIS_DATA_REG			0x02
#define BMA220_CHIP_ID_REG				0x00



/** Definitions of bit slices 
*/


/** CHIP_ID_REG
*/

/** CHIP_ID
*/
#define BMA220_CHIP_ID__POS				0
#define BMA220_CHIP_ID__MSK				0xFF
#define BMA220_CHIP_ID__LEN				8
#define BMA220_CHIP_ID__REG				BMA220_CHIP_ID_REG


/** X_AXIS_DATA_REG
*/

/** ACC_X
*/
#define BMA220_ACC_X__POS				0
#define BMA220_ACC_X__MSK				0xFF
#define BMA220_ACC_X__LEN				8
#define BMA220_ACC_X__REG				BMA220_X_AXIS_DATA_REG


/** Y_AXIS_DATA_REG
*/

/** ACC_Y
*/
#define BMA220_ACC_Y__POS				0
#define BMA220_ACC_Y__MSK				0xFF
#define BMA220_ACC_Y__LEN				8
#define BMA220_ACC_Y__REG				BMA220_Y_AXIS_DATA_REG


/** Z_AXIS_DATA_REG
*/

/** ACC_Z
*/
#define BMA220_ACC_Z__POS				0
#define BMA220_ACC_Z__MSK				0xFF
#define BMA220_ACC_Z__LEN				8
#define BMA220_ACC_Z__REG				BMA220_Z_AXIS_DATA_REG


/** HG_CONFIG_REG
*/

/** HIGH_DUR
*/
#define BMA220_HIGH_DUR__POS			0
#define BMA220_HIGH_DUR__MSK			0x3F
#define BMA220_HIGH_DUR__LEN			6
#define BMA220_HIGH_DUR__REG			BMA220_HG_CONFIG_REG

/** HIGH_HY
*/
#define BMA220_HIGH_HY__POS				6
#define BMA220_HIGH_HY__MSK				0xC0
#define BMA220_HIGH_HY__LEN				2
#define BMA220_HIGH_HY__REG				BMA220_HG_CONFIG_REG


/** HG_LG_THRESHOLD_REG
*/

/** LOW_TH
*/
#define BMA220_LOW_TH__POS				4
#define BMA220_LOW_TH__MSK				0xF0
#define BMA220_LOW_TH__LEN				4
#define BMA220_LOW_TH__REG				BMA220_HG_LG_THRESHOLD_REG

/** HIGH_TH
*/
#define BMA220_HIGH_TH__POS				0
#define BMA220_HIGH_TH__MSK				0x0F
#define BMA220_HIGH_TH__LEN				4
#define BMA220_HIGH_TH__REG				BMA220_HG_LG_THRESHOLD_REG


/** LG_CONFIG_REG
*/

/** LOW_DUR
*/
#define BMA220_LOW_DUR__POS				0
#define BMA220_LOW_DUR__MSK				0x3F
#define BMA220_LOW_DUR__LEN				6
#define BMA220_LOW_DUR__REG				BMA220_LG_CONFIG_REG

/** LOW_HY
*/
#define BMA220_LOW_HY__POS				6
#define BMA220_LOW_HY__MSK				0xC0
#define BMA220_LOW_HY__LEN				2
#define BMA220_LOW_HY__REG				BMA220_LG_CONFIG_REG


/** TAP_SENSING_CONFIG_REG
*/

/** TT_DUR
*/
#define BMA220_TT_DUR__POS				0
#define BMA220_TT_DUR__MSK				0x07
#define BMA220_TT_DUR__LEN				3
#define BMA220_TT_DUR__REG				BMA220_TAP_SENSING_CONFIG_REG

/** TT_TH
*/
#define BMA220_TT_TH__POS				3
#define BMA220_TT_TH__MSK				0x78
#define BMA220_TT_TH__LEN				4
#define BMA220_TT_TH__REG				BMA220_TAP_SENSING_CONFIG_REG

/** TT_FILT
*/
#define BMA220_TT_FILT__POS				7
#define BMA220_TT_FILT__MSK				0x80
#define BMA220_TT_FILT__LEN				1
#define BMA220_TT_FILT__REG				BMA220_TAP_SENSING_CONFIG_REG


/** SLOPE_DET_CONFIG1_REG
*/

/** SLOPE_DUR
*/
#define BMA220_SLOPE_DUR__POS			0
#define BMA220_SLOPE_DUR__MSK			0x03
#define BMA220_SLOPE_DUR__LEN			2
#define BMA220_SLOPE_DUR__REG			BMA220_SLOPE_DET_CONFIG_REG

/** SLOPE_TH 
*/
#define BMA220_SLOPE_TH__POS			2
#define BMA220_SLOPE_TH__MSK			0x3C
#define BMA220_SLOPE_TH__LEN			4
#define BMA220_SLOPE_TH__REG			BMA220_SLOPE_DET_CONFIG_REG

/** SLOPE_FILT
*/
#define BMA220_SLOPE_FILT__POS			6
#define BMA220_SLOPE_FILT__MSK			0x40
#define BMA220_SLOPE_FILT__LEN			1
#define BMA220_SLOPE_FILT__REG			BMA220_SLOPE_DET_CONFIG_REG


/** ORIENT_EX
*/
#define BMA220_ORIENT_EX__POS			7
#define BMA220_ORIENT_EX__MSK			0x80
#define BMA220_ORIENT_EX__LEN			1
#define BMA220_ORIENT_EX__REG			BMA220_SLOPE_DET_CONFIG_REG


/** DIRECTION_CONFIG_REG
*/

/** TT_SAMP
*/
#define BMA220_TT_SAMP__POS				0
#define BMA220_TT_SAMP__MSK				0x03
#define BMA220_TT_SAMP__LEN				2
#define BMA220_TT_SAMP__REG				BMA220_DIRECTION_CONFIG_REG

/** ORIENT_BLOCKING
*/
#define BMA220_ORIENT_BLOCKING__POS		2
#define BMA220_ORIENT_BLOCKING__MSK		0x0C
#define BMA220_ORIENT_BLOCKING__LEN		2
#define BMA220_ORIENT_BLOCKING__REG		BMA220_DIRECTION_CONFIG_REG

/** TIP_EN
*/
#define BMA220_TIP_EN__POS				4
#define BMA220_TIP_EN__MSK				0x10
#define BMA220_TIP_EN__LEN				1
#define BMA220_TIP_EN__REG				BMA220_DIRECTION_CONFIG_REG


/** DIRECTION_STATUS_REG
*/

/** INT_SIGN
*/
#define BMA220_INT_SIGN__POS			0
#define BMA220_INT_SIGN__MSK			0x01
#define BMA220_INT_SIGN__LEN			1
#define BMA220_INT_SIGN__REG			BMA220_DIRECTION_STATUS_REG

/** INT_FIRST_Z
*/
#define BMA220_INT_FIRST_Z__POS			1
#define BMA220_INT_FIRST_Z__MSK			0x02
#define BMA220_INT_FIRST_Z__LEN			1
#define BMA220_INT_FIRST_Z__REG			BMA220_DIRECTION_STATUS_REG

/** INT_FIRST_Y
*/
#define BMA220_INT_FIRST_Y__POS			2
#define BMA220_INT_FIRST_Y__MSK			0x04
#define BMA220_INT_FIRST_Y__LEN			1
#define BMA220_INT_FIRST_Y__REG			BMA220_DIRECTION_STATUS_REG

/** INT_FIRST_X
*/
#define BMA220_INT_FIRST_X__POS			3
#define BMA220_INT_FIRST_X__MSK			0x08
#define BMA220_INT_FIRST_X__LEN			1
#define BMA220_INT_FIRST_X__REG			BMA220_DIRECTION_STATUS_REG

/** INT_FIRST_XYZ
*/
#define BMA220_INT_FIRST_XYZ__POS			1
#define BMA220_INT_FIRST_XYZ__MSK			0x0E
#define BMA220_INT_FIRST_XYZ__LEN			3
#define BMA220_INT_FIRST_XYZ__REG			BMA220_DIRECTION_STATUS_REG

/** ORIENT
*/
#define BMA220_ORIENT__POS				4
#define BMA220_ORIENT__MSK				0x70
#define BMA220_ORIENT__LEN				3
#define BMA220_ORIENT__REG				BMA220_DIRECTION_STATUS_REG

/** ORIENT_INT
*/
#define BMA220_ORIENT_INT__POS			7
#define BMA220_ORIENT_INT__MSK			0x80
#define BMA220_ORIENT_INT__LEN			1
#define BMA220_ORIENT_INT__REG			BMA220_DIRECTION_STATUS_REG


/** INT_STATUS_REG
*/

/** SLOPE_INT
*/
#define BMA220_SLOPE_INT__POS			0
#define BMA220_SLOPE_INT__MSK			0x01
#define BMA220_SLOPE_INT__LEN			1
#define BMA220_SLOPE_INT__REG			BMA220_INT_STATUS_REG

/** DATA_INT
*/
#define BMA220_DATA_INT__POS			1
#define BMA220_DATA_INT__MSK			0x02
#define BMA220_DATA_INT__LEN			1
#define BMA220_DATA_INT__REG			BMA220_INT_STATUS_REG

/** HIGH_INT
*/
#define BMA220_HIGH_INT__POS			2
#define BMA220_HIGH_INT__MSK			0x04
#define BMA220_HIGH_INT__LEN			1
#define BMA220_HIGH_INT__REG			BMA220_INT_STATUS_REG

/** LOW_INT
*/
#define BMA220_LOW_INT__POS				3
#define BMA220_LOW_INT__MSK				0x08
#define BMA220_LOW_INT__LEN				1
#define BMA220_LOW_INT__REG				BMA220_INT_STATUS_REG

/** TT_INT
*/
#define BMA220_TT_INT__POS				4
#define BMA220_TT_INT__MSK				0x10
#define BMA220_TT_INT__LEN				1
#define BMA220_TT_INT__REG				BMA220_INT_STATUS_REG


/** INT_ENABLE1_REG
*/

/** EN_TT_Z
*/
#define BMA220_EN_TT_Z__POS				0
#define BMA220_EN_TT_Z__MSK				0x01
#define BMA220_EN_TT_Z__LEN				1
#define BMA220_EN_TT_Z__REG				BMA220_INT_ENABLE1_REG

/** EN_TT_Y
*/
#define BMA220_EN_TT_Y__POS				1
#define BMA220_EN_TT_Y__MSK				0x02
#define BMA220_EN_TT_Y__LEN				1
#define BMA220_EN_TT_Y__REG				BMA220_INT_ENABLE1_REG

/** EN_TT_X
*/
#define BMA220_EN_TT_X__POS				2
#define BMA220_EN_TT_X__MSK				0x04
#define BMA220_EN_TT_X__LEN				1
#define BMA220_EN_TT_X__REG				BMA220_INT_ENABLE1_REG


/** EN_TT_XYZ
*/
#define BMA220_EN_TT_XYZ__POS			0
#define BMA220_EN_TT_XYZ__MSK			0x07
#define BMA220_EN_TT_XYZ__LEN	  		3
#define BMA220_EN_TT_XYZ__REG			BMA220_INT_ENABLE1_REG


/** EN_SLOPE_Z
*/
#define BMA220_EN_SLOPE_Z__POS			3
#define BMA220_EN_SLOPE_Z__MSK			0x08
#define BMA220_EN_SLOPE_Z__LEN			1
#define BMA220_EN_SLOPE_Z__REG			BMA220_INT_ENABLE1_REG

/** EN_SLOPE_Y
*/
#define BMA220_EN_SLOPE_Y__POS			4
#define BMA220_EN_SLOPE_Y__MSK			0x10
#define BMA220_EN_SLOPE_Y__LEN			1
#define BMA220_EN_SLOPE_Y__REG			BMA220_INT_ENABLE1_REG

/** EN_SLOPE_X
*/
#define BMA220_EN_SLOPE_X__POS			5
#define BMA220_EN_SLOPE_X__MSK			0x20
#define BMA220_EN_SLOPE_X__LEN			1
#define BMA220_EN_SLOPE_X__REG			BMA220_INT_ENABLE1_REG


/** EN_SLOPE_XYZ
*/
#define BMA220_EN_SLOPE_XYZ__POS		3
#define BMA220_EN_SLOPE_XYZ__MSK		0x38
#define BMA220_EN_SLOPE_XYZ__LEN	  	3
#define BMA220_EN_SLOPE_XYZ__REG		BMA220_INT_ENABLE1_REG


/** EN_ORIENT
*/
#define BMA220_EN_ORIENT__POS			6
#define BMA220_EN_ORIENT__MSK			0x40
#define BMA220_EN_ORIENT__LEN			1
#define BMA220_EN_ORIENT__REG			BMA220_INT_ENABLE1_REG

/** EN_DATA
*/
#define BMA220_EN_DATA__POS				7
#define BMA220_EN_DATA__MSK				0x80
#define BMA220_EN_DATA__LEN				1
#define BMA220_EN_DATA__REG				BMA220_INT_ENABLE1_REG


/** INT_ENABLE2_REG
*/

/** EN_HIGH_Z
*/
#define BMA220_EN_HIGH_Z__POS			0
#define BMA220_EN_HIGH_Z__MSK			0x01
#define BMA220_EN_HIGH_Z__LEN			1
#define BMA220_EN_HIGH_Z__REG			BMA220_INT_ENABLE2_REG

/** EN_HIGH_Y
*/
#define BMA220_EN_HIGH_Y__POS			1
#define BMA220_EN_HIGH_Y__MSK			0x02
#define BMA220_EN_HIGH_Y__LEN			1
#define BMA220_EN_HIGH_Y__REG			BMA220_INT_ENABLE2_REG

/** EN_HIGH_X
*/
#define BMA220_EN_HIGH_X__POS			2
#define BMA220_EN_HIGH_X__MSK			0x04
#define BMA220_EN_HIGH_X__LEN			1
#define BMA220_EN_HIGH_X__REG			BMA220_INT_ENABLE2_REG


/** EN_HIGH_X
*/
#define BMA220_EN_HIGH_XYZ__POS			0
#define BMA220_EN_HIGH_XYZ__MSK			0x07
#define BMA220_EN_HIGH_XYZ__LEN			3
#define BMA220_EN_HIGH_XYZ__REG			BMA220_INT_ENABLE2_REG

/** EN_LOW
*/
#define BMA220_EN_LOW__POS				3
#define BMA220_EN_LOW__MSK				0x08
#define BMA220_EN_LOW__LEN				1
#define BMA220_EN_LOW__REG				BMA220_INT_ENABLE2_REG

/** LAT_INT 
*/
#define BMA220_LATCH_INT__POS			4
#define BMA220_LATCH_INT__MSK			0x70
#define BMA220_LATCH_INT__LEN		  	3
#define BMA220_LATCH_INT__REG			BMA220_INT_ENABLE2_REG

/** RESET_INT 
*/
#define BMA220_RESET_INT__POS			7
#define BMA220_RESET_INT__MSK			0x80
#define BMA220_RESET_INT__LEN	  		1
#define BMA220_RESET_INT__REG			BMA220_INT_ENABLE2_REG


/** SLEEP_CONFIG_REG
*/

/** EN_Z_CHANNEL
*/
#define BMA220_EN_Z_CHANNEL__POS		0
#define BMA220_EN_Z_CHANNEL__MSK		0x01
#define BMA220_EN_Z_CHANNEL__LEN		1
#define BMA220_EN_Z_CHANNEL__REG		BMA220_SLEEP_CONFIG_REG

/** EN_Y_CHANNEL
*/
#define BMA220_EN_Y_CHANNEL__POS		1
#define BMA220_EN_Y_CHANNEL__MSK		0x02
#define BMA220_EN_Y_CHANNEL__LEN		1
#define BMA220_EN_Y_CHANNEL__REG		BMA220_SLEEP_CONFIG_REG

/** EN_X_CHANNEL
*/
#define BMA220_EN_X_CHANNEL__POS		2
#define BMA220_EN_X_CHANNEL__MSK		0x04
#define BMA220_EN_X_CHANNEL__LEN		1
#define BMA220_EN_X_CHANNEL__REG		BMA220_SLEEP_CONFIG_REG

/** EN_XYZ_CHANNEL
*/
#define BMA220_EN_XYZ_CHANNEL__POS		0
#define BMA220_EN_XYZ_CHANNEL__MSK		0x07
#define BMA220_EN_XYZ_CHANNEL__LEN		3
#define BMA220_EN_XYZ_CHANNEL__REG		BMA220_SLEEP_CONFIG_REG

/** SLEEP_DUR
*/
#define BMA220_SLEEP_DUR__POS			3
#define BMA220_SLEEP_DUR__MSK			0x38
#define BMA220_SLEEP_DUR__LEN	  		3
#define BMA220_SLEEP_DUR__REG			BMA220_SLEEP_CONFIG_REG

/** SLEEP_EN
*/
#define BMA220_SLEEP_EN__POS			6
#define BMA220_SLEEP_EN__MSK			0x40
#define BMA220_SLEEP_EN__LEN	  		1
#define BMA220_SLEEP_EN__REG			BMA220_SLEEP_CONFIG_REG


/** BANDWIDTH_CONFIG_REG
*/

/** SC_FILT_CONFIG
*/
#define BMA220_SC_FILT_CONFIG__POS		0
#define BMA220_SC_FILT_CONFIG__MSK		0x0F
#define BMA220_SC_FILT_CONFIG__LEN	  	4
#define BMA220_SC_FILT_CONFIG__REG		BMA220_BANDWIDTH_CONFIG_REG

/** EXT_FILT_CONFIG
*/
//#define BMA220_EXT_FILT_CONFIG__POS		3
//#define BMA220_EXT_FILT_CONFIG__MSK		0x38
//#define BMA220_EXT_FILT_CONFIG__LEN		3
//#define BMA220_EXT_FILT_CONFIG__REG		BMA220_BANDWIDTH_CONFIG_REG

/** RC_FILT_EN
*/
//#define BMA220_RC_FILT_EN__POS			6
//#define BMA220_RC_FILT_EN__MSK			0x40
//#define BMA220_RC_FILT_EN__LEN			1
//#define BMA220_RC_FILT_EN__REG			BMA220_BANDWIDTH_CONFIG_REG

/** SERIAL_HIGH_BW
*/
#define BMA220_SERIAL_HIGH_BW__POS		7
#define BMA220_SERIAL_HIGH_BW__MSK		0x80
#define BMA220_SERIAL_HIGH_BW__LEN		1
#define BMA220_SERIAL_HIGH_BW__REG		BMA220_BANDWIDTH_CONFIG_REG


/** RANGE_SELFTEST_REG 
*/

/** RANGE
*/
#define BMA220_RANGE__POS				0
#define BMA220_RANGE__MSK				0x03
#define BMA220_RANGE__LEN				2
#define BMA220_RANGE__REG				BMA220_RANGE_SELFTEST_REG

/** SBIST
*/
#define BMA220_SBIST__POS				2
#define BMA220_SBIST__MSK				0x0C
#define BMA220_SBIST__LEN				2
#define BMA220_SBIST__REG				BMA220_RANGE_SELFTEST_REG

/** SBIST_SIGN
*/
#define BMA220_SBIST_SIGN__POS			4
#define BMA220_SBIST_SIGN__MSK			0x10
#define BMA220_SBIST_SIGN__LEN			1
#define BMA220_SBIST_SIGN__REG			BMA220_RANGE_SELFTEST_REG


/** HPASS_CONFIG_REG
*/

/** HP_Z_EN
*/
#define BMA220_HP_Z_EN__POS				0
#define BMA220_HP_Z_EN__MSK				0x01
#define BMA220_HP_Z_EN__LEN				1
#define BMA220_HP_Z_EN__REG				BMA220_HPASS_CONFIG_REG

/** HP_Y_EN
*/
#define BMA220_HP_Y_EN__POS				1
#define BMA220_HP_Y_EN__MSK				0x02
#define BMA220_HP_Y_EN__LEN				1
#define BMA220_HP_Y_EN__REG				BMA220_HPASS_CONFIG_REG

/** HP_X_EN
*/
#define BMA220_HP_X_EN__POS				2
#define BMA220_HP_X_EN__MSK				0x04
#define BMA220_HP_X_EN__LEN				1
#define BMA220_HP_X_EN__REG				BMA220_HPASS_CONFIG_REG

/** HP_XYZ_EN
*/
#define BMA220_HP_XYZ_EN__POS			0
#define BMA220_HP_XYZ_EN__MSK			0x07
#define BMA220_HP_XYZ_EN__LEN	  		3
#define BMA220_HP_XYZ_EN__REG			BMA220_HPASS_CONFIG_REG

/** HP_RESET
*/
#define BMA220_OFFSET_RESET__POS		3
#define BMA220_OFFSET_RESET__MSK		0x08
#define BMA220_OFFSET_RESET__LEN		1
#define BMA220_OFFSET_RESET__REG		BMA220_HPASS_CONFIG_REG

/** CAL_RDY
*/
#define BMA220_CAL_RDY__POS				4
#define BMA220_CAL_RDY__MSK				0x10
#define BMA220_CAL_RDY__LEN				1
#define BMA220_CAL_RDY__REG				BMA220_HPASS_CONFIG_REG

/** CAL_TRIGGER
*/
#define BMA220_CAL_TRIGGER__POS			5
#define BMA220_CAL_TRIGGER__MSK			0xE0
#define BMA220_CAL_TRIGGER__LEN			3
#define BMA220_CAL_TRIGGER__REG			BMA220_HPASS_CONFIG_REG


/** OFFSET_TARGET_REG
*/

/** CUT_OFF
*/
#define BMA220_CUT_OFF__POS				0
#define BMA220_CUT_OFF__MSK				0x01
#define BMA220_CUT_OFF__LEN				1
#define BMA220_CUT_OFF__REG				BMA220_OFFSET_TARGET_REG

/** OFFSET_TARGET_Z
*/
#define BMA220_OFFSET_TARGET_Z__POS		1
#define BMA220_OFFSET_TARGET_Z__MSK		0x06
#define BMA220_OFFSET_TARGET_Z__LEN		2
#define BMA220_OFFSET_TARGET_Z__REG		BMA220_OFFSET_TARGET_REG

/** OFFSET_TARGET_Y
*/
#define BMA220_OFFSET_TARGET_Y__POS		3
#define BMA220_OFFSET_TARGET_Y__MSK		0x18
#define BMA220_OFFSET_TARGET_Y__LEN		2
#define BMA220_OFFSET_TARGET_Y__REG		BMA220_OFFSET_TARGET_REG

/** OFFSET_TARGET_X
*/
#define BMA220_OFFSET_TARGET_X__POS		5
#define BMA220_OFFSET_TARGET_X__MSK		0x60
#define BMA220_OFFSET_TARGET_X__LEN		2
#define BMA220_OFFSET_TARGET_X__REG		BMA220_OFFSET_TARGET_REG

/** CAL_MANUAL
*/
#define BMA220_CAL_MANUAL__POS			7
#define BMA220_CAL_MANUAL__MSK			0x80
#define BMA220_CAL_MANUAL__LEN			1
#define BMA220_CAL_MANUAL__REG			BMA220_OFFSET_TARGET_REG

/** OFFSET_X_RESULT_REG
*/

/** OFFSET_X
*/
#define BMA220_OFFSET_X__POS			2
#define BMA220_OFFSET_X__MSK			0xFC
#define BMA220_OFFSET_X__LEN			6
#define BMA220_OFFSET_X__REG			BMA220_OFFSET_X_RESULT_REG


/** OFFSET_X_RESULT_REG
*/

/** OFFSET_Y
*/
#define BMA220_OFFSET_Y__POS			2
#define BMA220_OFFSET_Y__MSK			0xFC
#define BMA220_OFFSET_Y__LEN			6
#define BMA220_OFFSET_Y__REG			BMA220_OFFSET_Y_RESULT_REG


/** OFFSET_Z_RESULT_REG
*/

/** OFFSET_Z
*/
#define BMA220_OFFSET_Z__POS			2
#define BMA220_OFFSET_Z__MSK			0xFC
#define BMA220_OFFSET_Z__LEN			6
#define BMA220_OFFSET_Z__REG			BMA220_OFFSET_Z_RESULT_REG


/** SPI3
*/
#define BMA220_SPI3__POS				0
#define BMA220_SPI3__MSK				0x01
#define BMA220_SPI3__LEN				1
#define BMA220_SPI3__REG				BMA220_SPI_MODE_SELECT_REG

/** WATCHDOG TIMEOUT SELECT
*/
#define BMA220_WDT_TO_SEL__POS			1
#define BMA220_WDT_TO_SEL__MSK			0x02
#define BMA220_WDT_TO_SEL__LEN			1
#define BMA220_WDT_TO_SEL__REG			BMA220_SPI_MODE_SELECT_REG

/** WATCHDOG TIMEOUT ENABLE
*/
#define BMA220_WDT_TO_EN__POS			2
#define BMA220_WDT_TO_EN__MSK			0x04
#define BMA220_WDT_TO_EN__LEN			1
#define BMA220_WDT_TO_EN__REG			BMA220_SPI_MODE_SELECT_REG

/** WATCHDOG TIMEOUT
*/
#define BMA220_WDT_TO__POS				1
#define BMA220_WDT_TO__MSK				0x06
#define BMA220_WDT_TO__LEN				2
#define BMA220_WDT_TO__REG				BMA220_SPI_MODE_SELECT_REG


/** SUSPEND_MODE_REG
*/

/** SUSPEND
*/
#define BMA220_SUSPEND__POS				0
#define BMA220_SUSPEND__MSK				0xFF
#define BMA220_SUSPEND__LEN	  			8
#define BMA220_SUSPEND__REG				BMA220_SUSPEND_MODE_REG


/** SOFTRESET_REG
*/

/** SOFTRESET
*/
#define BMA220_SOFTRESET__POS			0
#define BMA220_SOFTRESET__MSK			0xFF
#define BMA220_SOFTRESET__LEN	  		8
#define BMA220_SOFTRESET__REG			BMA220_SOFTRESET_REG



/** register value bit slice constants definitions 
*/

/** RANGE setting
*/
#define BMA220_RANGE_2G					0x00
#define BMA220_RANGE_4G					0x01
#define BMA220_RANGE_8G					0x02
#define BMA220_RANGE_16G				0x03



/** BANDWIDTH constants for internal SC filter 
	\note when setting RC_FILT_EN is set to '1' the setting of internal BW is set to 1kHz
*/
#define BMA220_BANDWIDTH_50HZ			0x05
#define BMA220_BANDWIDTH_75HZ			0x04
#define BMA220_BANDWIDTH_150HZ			0x03
#define BMA220_BANDWIDTH_250HZ			0x02
#define BMA220_BANDWIDTH_600HZ			0x01
#define BMA220_BANDWIDTH_1000HZ			0x00


/** bandwidth settings for data rate selection while using external lowpass
using BMA220_EXT_FILT_AUTO is using maximum data rate to sample acceleration data.
\see RC_FILT_EN
\note RC_FILT_EN must be set '1' to use external filter with appropriate data rate
*/
#define BMA220_EXT_FILT_AUTO			0x00
#define BMA220_EXT_FILT_500Hz			0x01
#define BMA220_EXT_FILT_400Hz			0x02
#define BMA220_EXT_FILT_200Hz			0x03
#define BMA220_EXT_FILT_40Hz			0x04
#define BMA220_EXT_FILT_30Hz			0x05
#define BMA220_EXT_FILT_20Hz			0x06
#define BMA220_EXT_FILT_10Hz			0x07


/** constants for sleep time settings
*/
#define BMA220_SLEEP_DUR_2MS			0x00
#define BMA220_SLEEP_DUR_10MS			0x01
#define BMA220_SLEEP_DUR_25MS			0x02
#define BMA220_SLEEP_DUR_50MS			0x03
#define BMA220_SLEEP_DUR_100MS			0x04
#define BMA220_SLEEP_DUR_500MS			0x05
#define BMA220_SLEEP_DUR_1000MS			0x06
#define BMA220_SLEEP_DUR_2000MS			0x07



/** SLOPE Interrupt constants 
*/

/** SLOPE_DUR 
		defines the number of consecutive slope data points above slope_th which are required to set the interrupt
*/
#define BMA220_SLOPE_DUR_1				0x00
#define BMA220_SLOPE_DUR_3				0x01
#define BMA220_SLOPE_DUR_5				0x02
#define BMA220_SLOPE_DUR_7				0x03


/** TAP_SENSING_DUR 
*/
#define BMA220_TT_DUR_50MS				0x00
#define BMA220_TT_DUR_75MS				0x01
#define BMA220_TT_DUR_100MS				0x02
#define BMA220_TT_DUR_150MS				0x03
#define BMA220_TT_DUR_250MS				0x04
#define BMA220_TT_DUR_500MS				0x05
#define BMA220_TT_DUR_750MS				0x06
#define BMA220_TT_DUR_1000MS			0x07


/** LATCH_INT_DUR
*/
#define BMA220_LATCH_INT_OFF			0x00
#define BMA220_LATCH_INT_250MS			0x01
#define BMA220_LATCH_INT_500MS			0x02
#define BMA220_LATCH_INT_1000MS			0x03
#define BMA220_LATCH_INT_2000MS			0x04
#define BMA220_LATCH_INT_4000MS			0x05
#define BMA220_LATCH_INT_8000MS			0x06
#define BMA220_LATCH_INT_ON				0x07



/** macro definition for real value to register value conversion

*/

/** this macro convert real G values into 4 bit register values
    \note no range checking is included
    \see HIGH_TH, LOW_TH, SLOPE_TH, TT_TH
*/
#define BMA220_THRESHOLD_IN_G(threshold, grange)\
	((threshold * 16) / grange)
	
	
/** this macro convert real G values into 2 bit hysteresis register values
    \note no range checking is included
    \see HIGH_HYS, LOW_HYS, SLOPE_HYS, TT_HYS
*/
#define BMA220_HYSTERESIS_IN_G(hysteresis, grange)\
	((threshold * 4) / grange)
	


/** MACROS DEFINTION for register bit slicing, read access */
#define BMA220_GET_BITSLICE(regvar, bitname)\
			(regvar & bitname##__MSK) >> bitname##__POS


/** MACROS DEFINTION for register bit slicing, write access */
#define BMA220_SET_BITSLICE(regvar, bitname, val)\
		  (regvar & ~bitname##__MSK) | ((val<<bitname##__POS)&bitname##__MSK)  


#define BMA220_MDELAY_DATA_TYPE	unsigned int


/** Define the calling convention of YOUR ADC read out routine.

	If your ADC function looks like this:

	read_my_adc(unsigned char channel, unsigned short *);

	The BMA220_RD_ADC_PTR would equal:

    #define	BMA220_RD_ADC_SINGLE_PTR int (* adc_read)(unsigned char, unsigned short *)
	
	Parameters can be mixed as needed refer to the \ref BMA220_RD_ADC_SINGLE_PTR  macro.
*/
#define BMA220_RD_ADC_SINGLE_PTR int (* adc_read_single)(unsigned char, unsigned short *)
#define BMA220_RD_ADC_MULTI_PTR int (* adc_read_multi)(unsigned short *)
		  
typedef struct bma220 {	
	unsigned char mode;												/**< stores current operating mode of the sensor */
	unsigned char chip_id;										/**< holds the sensors chip_id after initialisation */
	unsigned char dev_addr;										/**< defines the I2C Address to be used */
	unsigned char int_mask;										/**< stores current interrupt enable mask */
	unsigned char interface;									/**<stores selected interface of bma220*/
	BMA220_WR_FUNC_PTR;												/**< reference to a pointer, pointing towards the bus write access function */
	BMA220_RD_FUNC_PTR;												/**< reference to a pointer, pointing towards the bus read access function */
	BMA220_RD_ADC_SINGLE_PTR;										/**< reference to a pointer, pointing towards the ADC signle channel access function */
	BMA220_RD_ADC_MULTI_PTR;										/**< reference to a pointer, pointing towards the ADC multi channel access function */
	void (*delay_msec)( BMA220_MDELAY_DATA_TYPE );   /**< reference to a pointer, pointing towards delay function */
} bma220_t;



		  
		  
typedef struct  {
	signed char x,	/**< holds x-axis acceleration data */
				y,	/**< holds y-axis acceleration data */
				z;	/**< holds z-axis acceleration data */
	
} bma220acc_t;


#define BMA220_CHANNEL_AX				0			/**< defines the ADC channel No. for x-axis */
#define BMA220_CHANNEL_AY				1			/**< defines the ADC channel No. for y-axis */
#define BMA220_CHANNEL_AZ				2			/**< defines the ADC channel No. for z-axis */

typedef struct	{
	unsigned short 	x,	/**< holds x-axis acceleration value in 16-bit counts of ADC*/
				   	y,	/**< holds y-axis acceleration value in 16-bit counts of ADC*/
				   	z;	/**< holds z-axis acceleration value in 16-bit counts of ADC*/
} bma220acc_analog_t; 



#define BMA220_I2C_ADDR					0x0B	

#define BMA220_DATA_SHIFT_RIGHT 		0x02


#define E_BMA_NULL_PTR					(char)-127
#define E_COMM_RES						(char)-1
#define E_OUT_OF_RANGE					(char)-2


int bma220_init(bma220_t * bma220);
int bma220_get_chip_id(unsigned char *chip_id);

/* Range configuration

*/
int bma220_set_range(unsigned char range);
int bma220_get_range(unsigned char * range);

/* Bandwidth configuration

*/
int bma220_set_bandwidth(char bw);
int bma220_get_bandwidth(unsigned char *bw);
int bma220_set_sc_filt_config(char sc_filt);
int bma220_get_sc_filt_config(unsigned char *sc_filt);

/* Mode configuration

*/
int bma220_set_mode(unsigned char mode);
int bma220_get_mode(unsigned char * mode);
int bma220_set_sleep_en(unsigned char sleep);
int bma220_get_sleep_en(unsigned char *sleep);
int bma220_set_sleep_dur(unsigned char sleep_dur);
int bma220_get_sleep_dur(unsigned char *sleep_dur);
int bma220_set_suspend(void);

/* Configuration (other)

*/
int bma220_soft_reset(void);

int bma220_set_spi3(unsigned char spi3);
int bma220_get_spi3(unsigned char *spi3);

int bma220_set_wdt_to_config(unsigned char wdt_to);
int bma220_get_wdt_to_config(unsigned char *wdt_to);

int bma220_set_serial_high_bw(unsigned char serial_high_bw);
int bma220_get_serial_high_bw(unsigned char *serial_high_bw);

/* Interrupt (other)

*/
int bma220_reset_int(void);

int bma220_set_latch_int(unsigned char latch_int);
int bma220_get_latch_int(unsigned char *latch_int);

int bma220_set_en_data(unsigned char en_data);
int bma220_get_en_data(unsigned char *en_data);

int bma220_set_interrupt_register(unsigned char int_enable2);
int bma220_set_direction_interrupt_register(unsigned char data);
int bma220_get_direction_status_register(unsigned char *direction_status_reg);
int bma220_get_interrupt_status_register(unsigned char *interrupt_status_reg);

/** high g interrupt configuration 

*/
int bma220_set_en_high_xyz(unsigned char en_high_xyz);
int bma220_get_en_high_xyz(unsigned char *en_high_xyz);
int bma220_set_high_th(unsigned char high_th);
int bma220_get_high_th(unsigned char *high_th);
int bma220_set_high_hy(unsigned char high_hy);
int bma220_get_high_hy(unsigned char *high_hy);
int bma220_set_high_dur(unsigned char high_dur);
int bma220_get_high_g_dur(unsigned char *high_dur);

/** low g interrupt configuration 

*/
int bma220_set_en_low(unsigned char en_low);
int bma220_get_en_low(unsigned char *en_low);
int bma220_set_low_th(unsigned char low_th);
int bma220_get_low_th(unsigned char *low_th);
int bma220_set_low_hy(unsigned char low_hy);
int bma220_get_low_hy(unsigned char *low_hy);
int bma220_set_low_dur(unsigned char low_dur);
int bma220_get_low_g_dur(unsigned char *low_dur);

/** Orientation configuration 

*/
int bma220_set_en_orient(unsigned char en_orient);
int bma220_get_orient_int(unsigned char *orient_int); 
int bma220_get_en_orient(unsigned char *en_orient);
int bma220_set_orient_ex(unsigned char orient_ex);
int bma220_get_orient_ex(unsigned char *orient_ex);
int bma220_set_orient_blocking(unsigned char orient_block);
int bma220_get_orient_blocking(unsigned char *orient_block);
int bma220_get_orientation(unsigned char * orient); 

/** Tap Sensing configuration 

*/
int bma220_set_en_tt_xyz(unsigned char en_tt_xyz);
int bma220_get_en_tt_xyz(unsigned char *en_tt_xyz);
int bma220_set_tt_th(unsigned char tt_th);
int bma220_get_tt_th(unsigned char *tt_th);
int bma220_set_tt_dur(unsigned char tt_dur);
int bma220_get_tt_dur(unsigned char *tt_dur);
int bma220_set_tt_filt(unsigned char tt_filt);
int bma220_get_tt_filt(unsigned char *tt_filt);
int bma220_set_tt_samp(unsigned char tt_samp);
int bma220_get_tt_samp(unsigned char *tt_samp);
int bma220_set_tip_en(unsigned char tip_en);
int bma220_get_tip_en(unsigned char *tip_en);

/** Slope interrupt configuration
	
*/
int bma220_set_en_slope_xyz(unsigned char en_slope_xyz);
int bma220_get_en_slope_xyz(unsigned char *en_slope_xyz);
int bma220_set_slope_th(unsigned char slope_th);
int bma220_get_slope_th(unsigned char *slope_th);
int bma220_set_slope_dur(unsigned char slope_dur);
int bma220_get_slope_dur(unsigned char *slope_dur);
int bma220_set_slope_filt(unsigned char slope_filt);
int bma220_get_slope_filt(unsigned char *slope_filt);

/* Selftest configuration

*/
int bma220_set_sbist(unsigned char sbist);
int bma220_get_sbist(unsigned char *sbist);
int bma220_set_sbist_sign(unsigned char sbist_sign);
int bma220_get_sbist_sign(unsigned char *sbist_sign);

/* Offset controller functions

*/
int bma220_set_offset_xyz(bma220acc_t offset); 
int bma220_get_offset_xyz(bma220acc_t *offset);
int bma220_set_cal_manual(unsigned char cal_manual);
int bma220_get_cal_manual(unsigned char *cal_manual);
int bma220_set_cal_trigger(unsigned char cal_trigger);
int bma220_get_cal_rdy(unsigned char * cal_rdy);
int bma220_set_offset_reset(unsigned char off_reset);
int bma220_set_hp_xyz_en(unsigned char hp_xyz_en);
int bma220_get_hp_xyz_en(unsigned char *hp_xyz_en);
int bma220_set_offset_target_x(unsigned char offset_target_x);
int bma220_get_offset_target_x(unsigned char *offset_target_x);
int bma220_set_offset_target_y(unsigned char offset_target_y);
int bma220_get_offset_target_y(unsigned char *offset_target_y);
int bma220_set_offset_target_z(unsigned char offset_target_z);
int bma220_get_offset_target_z(unsigned char *offset_target_z);
int bma220_set_cut_off_speed(unsigned char cut_off_speed);
int bma220_get_cut_off_speed(unsigned char *cut_off_speed);

/* Acceleration related functions

*/
int bma220_read_accel_x(signed char  * accel_x);
int bma220_read_accel_y(signed char  * accel_y);
int bma220_read_accel_z(signed char  * accel_z);
int bma220_read_accel_xyz(bma220acc_t * accel);

/* Direct access to registers

*/
int bma220_get_reg(unsigned char , unsigned char *, unsigned char);
int bma220_set_reg(unsigned char , unsigned char*, unsigned char );

#endif //__BMA220_H__
