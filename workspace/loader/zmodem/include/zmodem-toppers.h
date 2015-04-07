/**
 * \brief 	      Receive a file over a serial port using ZMODEM protocol and store it into a buffer.
 * \param  portid ID of the serial port for receiving
 * \param  buf    Pointer of the buffer to store received file
 * \param  size   Size of the buffer
 * \param  filesz Pointer to store the size of received file
 * \retval E_OK   Success
 */
extern ER zmodem_recv_file(ID portid, void *buf, SIZE size, SIZE *filesz);
