/*****************************************************************
"Copyright ? 2019-present Lenovo
Licensed under BSD-3, see COPYING.BSD file for details."
*****************************************************************/


#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/ioctl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <limits.h>
#include <systemd/sd-bus.h>
#include "fpga.h"


int fd;
local_fpga_t lfpga;

int open_dev(void)
{
	int err = -1;
	fd = open(FPGA_DRV,O_RDWR);
	if( fd < 0 ){
		err = 0;
		return err;
	}
	return fd;
}

int close_dev(int fd)
{
	int rc = -1;
	rc = close(fd);
	return rc;
}

/*                                                              
 * Returns 1 if the FPGA supports the requested block#.
 */
static uint8_t fpga_supports_block( common_fpga_t *info, uint16_t block )
{
    int i;

    if ( info == (common_fpga_t *)0 )
        return 0;

    if ( (info->present == 0) || ( info->caps == NULL ) )
        return 0;

    for( i = 0; info->caps[i].block != 0xFF; i++ )
    {
        if ( info->caps[i].block == block )
        {
            if ( info->caps[i].always )
                return 1;

            return (info->block0.data[ info->caps[i].offset ] & (1 << info->caps[i].bit)) ? 1 : 0;
        }
    }

    return 0;
}

/*                                                              
 * Generate a LUT, block_address, for quick translation of block# to absolute FPGA address.
*/
static void calculate_block_addresses( common_fpga_t *info, block_t *block0 )
{
    uint8_t block_id;
    uint16_t index = 0;

    if ( info == (common_fpga_t *)0 )
        return;

    /* cache block0 data */
    memcpy( (void *)info->block0.data, (void *)block0, FPGA_BLOCK_SIZE );

    /* compute block addresses */
    index = FPGA_BLOCK_SIZE;
    info->block_addresses[ 0 ] = 0;
    for( block_id = 1; block_id < FPGA_MAX_BLOCKS; block_id++ )
    {
        if ( fpga_supports_block( info, block_id ) == 0 )
        {
            info->block_addresses[ block_id ] = ~0;
            continue;
        }

        info->block_addresses[ block_id ] = index;
        index += FPGA_BLOCK_SIZE;
    }
}


static uint8_t fpga_page_size( uint16_t size )
{
    return (size >> 4);
}

uint16_t fpga_block_to_addr( common_fpga_t *info, uint8_t block_id )
{
    if ( !fpga_supports_block( info, block_id ) )
        return ~0;

    return info->block_addresses[ block_id ];
}

int fpga_transfer(spiInfo *info)
{
	struct spi_ioc_transfer xfer[1];
	int i = 0;
	int rc = 0;
	uint8_t rx_len = 0,rw_flag = 0;
	
	//printf("before transfer %u\n",info->tx_len);
	/*******************************************
	* bit7 bit6 bit5 bit4 bit3 bit2 bit1 bit0
	* RW   ADI  |  00    |  RMT   |    00   |
	*  0  |      ID           | 0 | ADDRESS |
	*|              ADDRESS                 |
	*|             Data Count               |
	*|             xData from BMC           |
	*|             CRC                      |
	*|             TAT                      |
	********************************************/
#if 0	
	for (i = 0; i< info->tx_len; i++)
	{
		printf("transfer data:%0x\n",info->tx_buff[i]);
	}
#endif
	rw_flag = info->tx_buff[0];
	rx_len = info->tx_buff[3];

	xfer[0].tx_nbits = 1;
	xfer[0].rx_nbits = 1;
	xfer[0].bits_per_word = 8;
	xfer[0].tx_buf = (unsigned long)info->tx_buff;
	xfer[0].rx_buf = (unsigned long)info->rx_buff;
	xfer[0].len = info->tx_len;
	if ((rc = ioctl(fd, SPI_IOC_MESSAGE(1), &xfer)) < 0) {
			perror("SPI_IOC_MESSAGE");
			
	}
	//printf("rx_len:%u\n",rx_len);
	if(!rw_flag){
		
		for (i = 0; i< rx_len; i++)
		{
			fprintf(stdout, "receive:%0x\n",info->rx_buff[i]);
		}
	}
	return 0;
}

static uint8_t compute_crc( uint16_t length, uint8_t *data )
{
    uint8_t crc[8] = {1,1,1,1,1,1,1,1};
    uint8_t d[8], cr[8];
    uint8_t big_crc = 0;
    int i;
    
    if ( data == (uint8_t *)0 )
        return big_crc;

    for( i = 0; i < length; i++ )
    {
        d[ 7 ] = (data[ i ] >> 7) & 1;
        d[ 6 ] = (data[ i ] >> 6) & 1;
        d[ 5 ] = (data[ i ] >> 5) & 1;
        d[ 4 ] = (data[ i ] >> 4) & 1;
        d[ 3 ] = (data[ i ] >> 3) & 1;
        d[ 2 ] = (data[ i ] >> 2) & 1;
        d[ 1 ] = (data[ i ] >> 1) & 1;
        d[ 0 ] = (data[ i ] >> 0) & 1;

        // poly = x^8 + x^2 + x^1 + x^0
        //        ---7----   ---6----   ---5----   ---4----   ---3----   ---2----   ---1----   ---0----
        cr[ 7 ] = crc[ 7 ]                                                        ^ crc[ 1 ] ^ crc[ 0 ] ;
        cr[ 6 ] = crc[ 7 ] ^ crc[ 6 ]                                             ^ crc[ 1 ]            ;
        cr[ 5 ] = crc[ 7 ] ^ crc[ 6 ] ^ crc[ 5 ]                                  ^ crc[ 1 ]            ;
        cr[ 4 ] =            crc[ 6 ] ^ crc[ 5 ] ^ crc[ 4 ]                                  ^ crc[ 0 ] ;
        cr[ 3 ] =                       crc[ 5 ] ^ crc[ 4 ] ^ crc[ 3 ]                                  ;
        cr[ 2 ] =                                  crc[ 4 ] ^ crc[ 3 ] ^ crc[ 2 ]                       ;
        cr[ 1 ] =                                             crc[ 3 ] ^ crc[ 2 ] ^ crc[ 1 ]            ;
        cr[ 0 ] =                                                        crc[ 2 ] ^ crc[ 1 ] ^ crc[ 0 ] ;

        //         --7---   --6---   --5---   --4---   --3---   --2---   --1---   --0---
        cr[ 7 ] ^= d[ 7 ]                                              ^ d[ 1 ] ^ d[ 0 ]  ;
        cr[ 6 ] ^= d[ 7 ] ^ d[ 6 ]                                     ^ d[ 1 ]           ;
        cr[ 5 ] ^= d[ 7 ] ^ d[ 6 ] ^ d[ 5 ]                            ^ d[ 1 ]           ;
        cr[ 4 ] ^=          d[ 6 ] ^ d[ 5 ] ^ d[ 4 ]                            ^ d[ 0 ]  ;
        cr[ 3 ] ^=                   d[ 5 ] ^ d[ 4 ] ^ d[ 3 ]                             ;
        cr[ 2 ] ^=                            d[ 4 ] ^ d[ 3 ] ^ d[ 2 ]                    ;
        cr[ 1 ] ^=                                     d[ 3 ] ^ d[ 2 ] ^ d[ 1 ]           ;
        cr[ 0 ] ^=                                              d[ 2 ] ^ d[ 1 ] ^ d[ 0 ]  ;


        // same as above, just different formatting
        //cr[ 7 ] = crc[ 7 ] ^ crc[ 1 ] ^ crc[ 0 ]            ^ d[ 7 ] ^ d[ 1 ] ^ d[ 0 ];
        //cr[ 6 ] = crc[ 7 ] ^ crc[ 6 ] ^ crc[ 1 ]            ^ d[ 7 ] ^ d[ 6 ] ^ d[ 1 ];
        //cr[ 5 ] = crc[ 7 ] ^ crc[ 6 ] ^ crc[ 5 ] ^ crc[ 1 ] ^ d[ 7 ] ^ d[ 6 ] ^ d[ 5 ] ^ d[ 1 ];
        //cr[ 4 ] = crc[ 6 ] ^ crc[ 5 ] ^ crc[ 4 ] ^ crc[ 0 ] ^ d[ 6 ] ^ d[ 5 ] ^ d[ 4 ] ^ d[ 0 ];
        //cr[ 3 ] = crc[ 5 ] ^ crc[ 4 ] ^ crc[ 3 ]            ^ d[ 5 ] ^ d[ 4 ] ^ d[ 3 ];
        //cr[ 2 ] = crc[ 4 ] ^ crc[ 3 ] ^ crc[ 2 ]            ^ d[ 4 ] ^ d[ 3 ] ^ d[ 2 ];
        //cr[ 1 ] = crc[ 3 ] ^ crc[ 2 ] ^ crc[ 1 ]            ^ d[ 3 ] ^ d[ 2 ] ^ d[ 1 ];
        //cr[ 0 ] = crc[ 2 ] ^ crc[ 1 ] ^ crc[ 0 ]            ^ d[ 2 ] ^ d[ 1 ] ^ d[ 0 ];

        crc[ 7 ] = cr[ 7 ];
        crc[ 6 ] = cr[ 6 ];
        crc[ 5 ] = cr[ 5 ];
        crc[ 4 ] = cr[ 4 ];
        crc[ 3 ] = cr[ 3 ];
        crc[ 2 ] = cr[ 2 ];
        crc[ 1 ] = cr[ 1 ];
        crc[ 0 ] = cr[ 0 ];
    }

    big_crc = (crc[7] << 7) | (crc[6] << 6) |
              (crc[5] << 5) | (crc[4] << 4) |
              (crc[3] << 3) | (crc[2] << 2) |
              (crc[1] << 1) | crc[0];

    return big_crc;
}

uint8_t fpga_calc_integrity_value( uint8_t *buf, uint16_t len, uint8_t type )
{
    uint8_t v = 0;

    if ( type == FPGA_INTEGRITY_CHECKSUM )
    {
        uint16_t chksum = 0x100;

        if ( buf != (uint8_t *)0 )
        {
            while( len-- )
                chksum -= buf[ len ];
        }

        v = (uint8_t)chksum;
    }
    else if ( type == FPGA_INTEGRITY_CRC7 )
    {
        v = compute_crc( len, buf );
    }

    return v;
}

int fpga_message_read(fpga_msg_t *msg)
{
    uint8_t in_buf[1032];
    uint8_t out_buf[1032];
	uint8_t bus;
    uint8_t cs;
    int adj = 0;
	spiInfo info;
	int len;
	bus = lfpga.spi_bus;
    cs = lfpga.spi_cs;
	out_buf[ 0 ]  = msg->adi ? 0x40 : 0;
    out_buf[ 0 ] |= (msg->rmt << 2);
    out_buf[ 1 ]  = (msg->addr >> 8) & 0x03;
    out_buf[ 1 ] |= msg->node_id << 3;
    out_buf[ 2 ]  = msg->addr & 0xFF;
    if ( msg->adi == 0 )
    {
        if ( msg->data_count >= 256 )
        {
            msg->data_count = 256;
            out_buf[ 3 ]  = 0;
        }
        else
            out_buf[ 3 ]  = msg->data_count;
        adj = 0;
		len = 4;
    }
    else
    {
        out_buf[ 3 ]  = msg->page >> 8;
        out_buf[ 4 ]  = msg->page;
        out_buf[ 5 ]  = msg->page_size;
        adj = 2;
    

    //memcpy( (void *)&out_buf[ 4 + adj ], (void *)msg->data, msg->data_count );
    //len = 4 + msg->data_count + adj;
    /*out_buf[ len ] = fpga_calc_integrity_value( out_buf,
                                                     len,
                                                     lfpga.common.integrity_type );*/
	}
    out_buf[ len ++ ] = 0;  //TAT
	memset( (void *)&info, 0, sizeof(spiInfo));
	info.num = bus;
    info.cs = cs;
    info.tx_buff = out_buf;
    info.rx_buff = in_buf;
    info.tx_len = len + adj;
    info.rx_len = msg->data_count + 2; // status, checksum
    info.add_mode = 0; // this shouldn't matter!
    info.spi_mem = 0;
    info.spi_mode = 1; // single bit SPI interface
    info.freq = lfpga.frequency;
	fpga_transfer(&info);
	memcpy((void*)msg->data,(void*)in_buf,msg->data_count);
	return 0;
}

int fpga_message_write(fpga_msg_t *msg)
{
    uint8_t in_buf[1032];
    uint8_t out_buf[1032];
	uint8_t bus;
    uint8_t cs;
    int adj = 0;
	spiInfo info;
	int len = 0;
	bus = lfpga.spi_bus;
    cs = lfpga.spi_cs;
	if ( msg->data_count > FPGA_MAX_DATA_XFER_SIZE ){
        msg->data_count = FPGA_MAX_DATA_XFER_SIZE;
	}
	out_buf[ 0 ]  = 0x80;
    out_buf[ 0 ] |= msg->adi ? 0x40 : 0;
    out_buf[ 0 ] |= (msg->rmt << 2);
    out_buf[ 1 ]  = (msg->addr >> 8) & 0x03;
    out_buf[ 1 ] |= msg->node_id << 3;
    out_buf[ 2 ]  = msg->addr & 0xFF;
    if ( msg->adi == 0 ){
        if ( msg->data_count >= 256 ){
            msg->data_count = 256;
            out_buf[ 3 ]  = 0;
        }else
            out_buf[ 3 ]  = msg->data_count;
        adj = 0;
    }else{
        out_buf[ 3 ]  = msg->page >> 8;
        out_buf[ 4 ]  = msg->page;
        out_buf[ 5 ]  = msg->page_size;
        adj = 2;
    }
    memcpy( (void *)&out_buf[ 4 + adj ], (void *)msg->data, msg->data_count );
    len = 4 + msg->data_count + adj;
    out_buf[ len ] = fpga_calc_integrity_value( out_buf,len,lfpga.common.integrity_type );
    len ++;
    out_buf[ len ++ ] = 0;  //TAT
	
	
	memset( (void *)&info, 0, sizeof(spiInfo));
	info.num = bus;
    info.cs = cs;
    info.tx_buff = out_buf;
    info.rx_buff = in_buf;
    info.tx_len = len + adj;
    info.rx_len = 2; // status, checksum
    info.add_mode = 0; // this shouldn't matter!
    info.spi_mem = 0;
    info.spi_mode = 1; // single bit SPI interface
    info.freq = lfpga.frequency;
	fpga_transfer(&info);
	return 0;
}

int fpga_read(fpga_cmd_t *cmd)
{
    fpga_msg_t *msg = NULL;
    int rc;
	//uint8_t *tmp = NULL;
    msg = &(lfpga.msg);
	//tmp = malloc(1024);
	//msg->data = tmp;
    memset( (void *)msg, 0, sizeof(fpga_msg_t));
    msg->adi        = cmd->adi;
    msg->mode       = FPGA_READ_MODE;
    msg->data_count = cmd->length;
    msg->rmt     = FPGA_ACCESS_LOCAL;
    msg->addr    = fpga_block_to_addr( &(lfpga.common), cmd->block );
    msg->node_id = 0x00;
	if ( msg->addr == (uint16_t)~0 )
        return -1;
	msg->addr += cmd->offset;
	if ( msg->adi == 1 )
    {
        if ( (cmd->length > 4096) || (cmd->length & 0x000F) )
        {

            //printf( "%s: requested page size incorrect: 0x%04X\n", __FUNCTION__, cmd->length );
            return -1;
        }

        msg->page_size = fpga_page_size( cmd->length );
        msg->page = cmd->page;
    }

    rc = fpga_message_read( msg );
    if ( rc == 0 )
        memcpy( (void *)cmd->data, (void *)msg->data, cmd->length );
	//printf("fpga read data:%0x,%x\n",cmd->data[0],msg->data[0]);
	//free(tmp);
    return rc;
}

int fpga_write(fpga_cmd_t *cmd)
{
    fpga_msg_t *msg = NULL;
    int rc;
    msg = &(lfpga.msg);
    memset( (void *)msg, 0, sizeof(fpga_msg_t));
    msg->adi        = cmd->adi;
    msg->mode       = FPGA_WRITE_MODE;
    msg->data_count = cmd->length;
    msg->rmt     = FPGA_ACCESS_LOCAL;
    msg->addr    = fpga_block_to_addr( &(lfpga.common), cmd->block );
    msg->node_id = 0x00;
	if ( msg->addr == (uint16_t)~0 )
        return -1;
	msg->addr += cmd->offset;
	if ( msg->adi == 1 )
    {
        if ( (cmd->length > 4096) || (cmd->length & 0x000F) )
        {

            //printf( "%s: requested page size incorrect: 0x%04X\n", __FUNCTION__, cmd->length );
            return -1;
        }

        msg->page_size = fpga_page_size( cmd->length );
        msg->page = cmd->page;
    }
    memcpy( (void *)msg->data, (void *)cmd->data, cmd->length );
    rc = fpga_message_write(msg);
    return rc;
}

int fpga_read_block(fpga_cmd_t *cmd )
{
    cmd->length = FPGA_BLOCK_SIZE;
    cmd->offset = 0;
    return fpga_read(cmd );
}

int altera_fpga_init(void)
{
	int err = -1;
	fpga_cmd_t *cmd;
	
	/*
	lfpga = malloc(sizeof(local_fpga_t));
	if(lfpga != NULL){
	    err = 0;
	}
	memset( (void *)lfpga, 0, sizeof(local_fpga_t) );
	*/
	lfpga.common.present = 1;
    
    lfpga.common.state = FPGA_STATE_INIT_IN_PROGRESS;
	lfpga.spi_bus                  = 0; // BMC BACKUP SPI
    lfpga.spi_cs                   = 0;
    lfpga.common.enforce_integrity = 1;
	lfpga.protocol_version         = 2;
    lfpga.common.integrity_type    = FPGA_INTEGRITY_CRC7;
    lfpga.frequency                = 4;  // 50MHz
	lfpga.common.caps = (fpga_caps_t *)&fpga_caps_v2[0];
    lfpga.common.block_mapper = (uint16_t *)&fpga_block_mapper_v2[0];
	cmd = &(lfpga.cmd);
	
	cmd->block  = lfpga.common.block_mapper[ FPGA_BLOCK_MAIN ];
	cmd->adi    = 0;
    cmd->action = FPGA_ACCESS_LOCAL;
	
   //fpga_read_block(cmd);

    /* cache block0 data for local FPGA */
    memcpy( (void *)lfpga.common.block0.data, (void *)cmd->data, FPGA_BLOCK_SIZE );
    calculate_block_addresses( &(lfpga.common), (block_t *)cmd->data );

	err = open_dev();
	return err;
}
#if 1
static int read_fpga(sd_bus_message *msg,fpga_cmd_t *t)
{
	int rc = -1;
	uint8_t block,offset;
		
	rc = sd_bus_message_read(msg, "yy", &block, &offset);
	t->action = 0;
	t->adi = 0; 
	t->block = block; 
	t->offset = offset; 
	t->length = 1;
	rc = fpga_read(t);
	return rc;
}
#if 1
static int write_fpga(sd_bus_message *msg)
{
	int rc = -1;
	uint8_t block,offset,data;
	fpga_cmd_t t;
	rc = sd_bus_message_read(msg, "yyy", &block, &offset,&data);
	t.action = 0;
	t.adi = 0;
	t.block = block;
	t.offset = offset;
	t.length = 1;
	
    memcpy((void*)t.data, (void*)&data,t.length); 
	rc = fpga_write(&t);
	return rc;
}
#endif
int fpga_function_router(sd_bus_message *msg, void *user_data,
		sd_bus_error *ret_error)
{
	/* Generic error reporter. */
	int rc = -1;
	int i = 0;
	fpga_cmd_t t;
	/* Get the Operation. */
	const char *fpga_function = sd_bus_message_get_member(msg);
	if (fpga_function == NULL) {
		fprintf(stderr, "fpgactl: Null FPGA function specified\n");
		return sd_bus_reply_method_return(msg, "i", rc);
	}
	/* Route the user action to appropriate handlers. */
	if ((strcmp(fpga_function, "read_fpga") == 0)) {
		rc = read_fpga(msg,&t);
		return sd_bus_reply_method_return(msg, "y",t.data[0]);
	}

#if 1
	if ((strcmp(fpga_function, "write_fpga") == 0)) {
		rc = write_fpga(msg);
		return sd_bus_reply_method_return(msg, "i", rc);
	}
#endif
	return sd_bus_reply_method_return(msg, "i", rc);
}


/*
 * -----------------------------------------------
 * Dbus Services offered by this fpga controller
 * -----------------------------------------------
 */
const sd_bus_vtable fpga_control_vtable[] =
{
	SD_BUS_VTABLE_START(0),
	SD_BUS_METHOD("read_fpga", "yy", "y", &fpga_function_router, SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_METHOD("write_fpga", "yyy", "i", &fpga_function_router, SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_VTABLE_END,
};

int start_fpga_services()
{
	/* Generic error reporter. */
	int rc = -1;
	/* slot where we are offering the FPGA dbus service. */
	sd_bus_slot *fpga_slot = NULL;
	sd_bus *bus = NULL;
	const char *fpga_object = "/org/openbmc/control/fpga";

	/* Get a hook onto system bus. */
	rc = sd_bus_open_system(&bus);
	if (rc < 0) {
		fprintf(stderr,"fpgactl: Error opening system bus.\n");
		return rc;
	}

	/* Install the object */
	rc = sd_bus_add_object_vtable(bus,
			&fpga_slot,
			fpga_object, /* object path */
			"org.openbmc.control.fpga", /* interface name */
			fpga_control_vtable,
			NULL);
	if (rc < 0) {
		fprintf(stderr, "fpgactl: Failed to add object to dbus: %s\n",
				strerror(-rc));
		return rc;
	}

	/* If we had success in adding the providers, request for a bus name. */
	rc = sd_bus_request_name(bus,
			"org.openbmc.control.fpga", 0);
	if (rc < 0) {
		fprintf(stderr, "fpgactl: Failed to acquire service name: %s\n",
				strerror(-rc));
		return rc;
	}

	for (;;) {
		/* Process requests */
		rc = sd_bus_process(bus, NULL);
		if (rc < 0) {
			fprintf(stderr, "fpgactl: Failed to process bus: %s\n",
					strerror(-rc));
			break;
		}
		if (rc > 0) {
			continue;
		}

		rc = sd_bus_wait(bus, (uint64_t) - 1);
		if (rc < 0) {
			fprintf(stderr, "fpgactl: Failed to wait on bus: %s\n",
				strerror(-rc));
			break;
		}
	}

	sd_bus_slot_unref(fpga_slot);
	sd_bus_unref(bus);

	return rc;
}
#endif
int main(int argc, char *argv[])
{
	int rc = 0;
	int c;
	altera_fpga_init();

	#if 1
	start_fpga_services();
	#endif
	//rc = close_dev(fd);
	return rc;
}
