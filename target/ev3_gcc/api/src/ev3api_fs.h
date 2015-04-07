/**
 * \file    ev3fs.h
 * \brief	API for EV3 file system
 * \author	ertl-liyixiao
 */

#pragma once

/**
 * \~English
 * [TODO: sync with jp version]
 * \defgroup ev3api-fs File system
 * \brief    Definitions of API for file system.
 *
 * \~Japanese
 * \defgroup ev3api-fs ファイルシステム
 * \brief    ファイルシステムに関するAPI．
 * \details  このページは本プラットフォーム独自のAPIしか記述しないが，fopen()等標準Cライブラリのファイル操作関数もサポートする．
 *
 * @{
 */

/**
 * \~English
 * \brief A memory file is a file stored in RAM rather than on the SD card.
 *
 * \~Japanese
 * \brief メモリファイルの構造体．メモリファイルのデータは，SDカードではなく，メモリに格納される．
 */
typedef struct {
	void*    buffer;   //!< \~English Pointer of the buffer where this file is stored. NULL means a invalid memory file. \~Japanese このファイルが格納されているバッファへのポインタ．NULLは無効なメモリファイル．
	uint32_t filesz;   //!< \~English Actual size of this file	 					                                     \~Japanese ファイルの実際のサイズ
	uint32_t buffersz; //!< \~English Maximum size of the buffer to store this file   									 \~Japanese バッファの最大サイズ
} memfile_t;

#define	TMAX_FILENAME_LEN (255)    //!< \~English Maximum length of a file name \~Japanese ファイル名の最大の長さ

/**
 * \~English
 * [TODO: sync with jp version]
 * \brief A memory file is a file stored in RAM rather than on the SD card.
 *
 * \~Japanese
 * \brief ファイル情報の構造体．
 */
typedef struct {
	uint32_t size;							//!< \~English File size          \~Japanese ファイルのサイズ
	uint16_t date;							//!< \~English Last modified date \~Japanese ファイルのLast modified date
	uint16_t time;							//!< \~English Last modified time \~Japanese ファイルのLast modified time
    bool_t   is_dir;                        //!< \~English Flag of a folder   \~Japanese フォルダであることを表すフラグ
    bool_t   is_readonly;                   //!< \~English Flag of read-only  \~Japanese 読み出し専用であることを表すフラグ
    bool_t   is_hidden;                     //!< \~English Flag of hidden     \~Japanese 隠しファイルであることを表すフラグ
	char	 name[TMAX_FILENAME_LEN + 1];	//!< \~English File name          \~Japanese ファイル名
} fileinfo_t;

/**
 * \~English
 * [TODO: sync with jp version]
 * \brief      Open a directory for reading.
 * \param path Path of the directory to be opened
 * \return     ID of the opened directory
 *
 * \~Japanese
 * \brief         ディレクトリをオープンする．
 * \details       成功した場合，オープンされたディレクトリのIDを戻り値として返す．このIDはディレクトリ内のファイル情報を取得するために使える．
 * \param  path   ディレクトリのパス
 * \retval >0     オープンされたディレクトリのID
 * \retval E_CTX  非タスクコンテストから呼び出す
 * \retval E_MACV メモリアクセス違反（path）
 * \retval E_NOID ID番号不足
 * \retval E_PAR  パス名は無効
 * \retval E_SYS  I/Oエラーが発生した（SDカード不良の可能性が高い）
 */
ER_ID ev3_sdcard_opendir(const char *path);

/**
 * \~English
 * [TODO: sync with jp version]
 * \brief             Close a directory.
 * \param  dirid      ID of an opened directory
 * \retval E_OK       Success
 *
 * \~Japanese
 * \brief             ディレクトリ内のファイル情報を読み込む．
 * \details           オープンされたディレクトリから次のファイルの情報を返す．
 * \param  dirid      ディレクトリのID
 * \param  p_fileinfo ファイル情報を格納する場所へのポインタ
 * \retval E_OK       正常終了
 * \retval E_CTX      非タスクコンテストから呼び出す
 * \retval E_ID       不正ID番号
 * \retval E_MACV     メモリアクセス違反（p_fileinfo）
 * \retval E_OBJ      これ以上読み込めるファイルの情報がない
 * \retval E_SYS  I/Oエラーが発生した（SDカード不良の可能性が高い）
 */
ER ev3_sdcard_readdir(ID dirid, fileinfo_t *p_fileinfo);

/**
 * \~English
 * [TODO: sync with jp version]
 * \brief             Close a directory.
 * \param  dirid      ID of an opened directory
 * \retval E_OK       Success
 *
 * \~Japanese
 * \brief          ディレクトリをクローズする．
 * \details        成功した場合，オープンされたディレクトリのリソースを解放して，そのIDは使えなくなる．
 * \param  dirid   ディレクトリのID
 * \retval E_OK    正常終了
 * \retval E_CTX   非タスクコンテストから呼び出す
 * \retval E_ID    不正ID番号
 * \retval E_SYS   I/Oエラーが発生した（SDカード不良の可能性が高い）
 */
ER ev3_sdcard_closedir(ID dirid);

/**
 * \~English
 * [TODO: sync with jp version]
 * \brief            Create a memory file and load a specific file into it from the SD card.
 * \param  path      Path of the file to be loaded
 * \param  p_memfile Pointer of save the created memory file
 * \retval E_OK      Success
 * \retval E_NOMEM   No enough free memory to create the memory file, or \a p_memfile is NULL. The \a buffer of \a p_memfile will be set to NULL if this happens.
 * \retval E_PAR     The \a path does not point to a valid file. The \a buffer of \a p_memfile will be set to NULL if this happens.
 * \retval E_OBJ     I/O failure, which might be caused by a corrupted SD card. The \a buffer of \a p_memfile will be set to NULL if this happens.
 *
 * \~Japanese
 * \brief            SDカードのファイルをメモリファイルとしてロードする．
 * \details          メモリファイルのオブジェクトを生成して，指定したファイルをSDカードからこのメモリファイルに読み込む．エラーが発生する場合，\a p_memfile の \a buffer はNULLにクリアする.
 * \param  path      ファイルのパス
 * \param  p_memfile 生成したメモリファイルのオブジェクトを格納する場所へのポインタ
 * \retval E_OK      正常終了
 * \retval E_MACV    メモリアクセス違反（pathかp_memfile）
 * \retval E_NOMEM   メモリ不足
 * \retval E_PAR     パス名は無効
 * \retval E_SYS     I/Oエラーが発生した（SDカード不良の可能性が高い）
 */
ER ev3_memfile_load(const char *path, memfile_t *p_memfile);

/**
 * \~English
 * [TODO: sync with jp version]
 * \brief            Free the resource (memory) allocated to a memory file. The \a buffer of \a p_memfile will be set to NULL on success.
 * \param  p_memfile Pointer of a memory file
 * \retval E_OK      Success
 * \retval E_PAR     The \a p_memfile is NULL.
 * \retval E_OBJ     The \a p_memfile does not point to a valid memory file.
 *
 * \~Japanese
 * \brief            メモリファイルを解放する．
 * \details          メモリファイルにより確保されたリソース（メモリ領域）を解放する．正常終了の場合，\a p_memfile の \a buffer はNULLにクリアする．
 * \param  p_memfile 解放するメモリファイルのポインタ
 * \retval E_OK      正常終了
 * \retval E_MACV    メモリアクセス違反（p_memfile）
 * \retval E_OBJ     \a p_memfile で指定したメモリファイルは無効
 */
ER ev3_memfile_free(memfile_t *p_memfile);

/**
 * \~English
 * [TODO: sync with jp version]
 *
 * \~Japanese
 * \brief シリアルポートを表す番号
 */
typedef enum {
    EV3_SERIAL_DEFAULT = 0, //!< \~English Default SIO port     \~Japanese デフォルトのシリアルポート（ログタスク用ポート）
    EV3_SERIAL_UART = 1,	//!< \~English UART (Sensor port 1) \~Japanese UARTポート（センサポート1）
    EV3_SERIAL_BT = 2,	    //!< \~English Bluetooth SPP        \~Japanese Bluetooth仮想シリアルポート
} serial_port_t;

/**
 * \~English
 * [TODO: sync with jp version]
 *
 * \~Japanese
 * \brief 	     シリアルポートをファイルとしてオープンする．
 * \details      失敗の場合，NULLを返す（エラーログが出力される）．
 * \param port   シリアルポート番号
 * \returns      シリアルポートのファイル
 */
FILE* ev3_serial_open_file(serial_port_t port);

/**
 * \~English
 * [TODO: sync with jp version]
 *
 * \~Japanese
 * \brief            Bluetooth (Serial Port Profile)が接続されているかどうかをチェックする．
 * \retval true      接続済み．Bluetooth仮想シリアルポートで通信できる．
 * \retval false     接続切れ．
 */
bool_t ev3_bluetooth_is_connected();

/**
 * @} // End of group
 */
