/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// DataChunk.cpp
// Implementation of Data Chunk save/load system
// Author: Michael S. Booth, October 2000

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "stdlib.h"
#include "Compression.h"
#include "Common/DataChunk.h"
#include "Common/file.h"
#include "Common/FileSystem.h"

// If verbose, lots of debug logging.
#define not_VERBOSE

CachedFileInputStream::CachedFileInputStream(void):m_buffer(nullptr),m_size(0)
{
}

CachedFileInputStream::~CachedFileInputStream(void)
{
	delete[] m_buffer;
	m_buffer=nullptr;
}

Bool CachedFileInputStream::open(AsciiString path)
{
	File *file=TheFileSystem->openFile(path.str(), File::READ | File::BINARY);
	m_size = 0;

	if (file) {
		m_size=file->size();
		if (m_size) {
			m_buffer = file->readEntireAndClose();
			file = nullptr;
		}
		m_pos=0;
	}

	if (CompressionManager::isDataCompressed(m_buffer, m_size) == 0)
	{
		//DEBUG_LOG(("CachedFileInputStream::open() - file %s is uncompressed at %d bytes!", path.str(), m_size));
	}
	else
	{
		Int uncompLen = CompressionManager::getUncompressedSize(m_buffer, m_size);
		//DEBUG_LOG(("CachedFileInputStream::open() - file %s is compressed!  It should go from %d to %d", path.str(),
		//	m_size, uncompLen));
		char *uncompBuffer = NEW char[uncompLen];
		Int actualLen = CompressionManager::decompressData(m_buffer, m_size, uncompBuffer, uncompLen);
		if (actualLen == uncompLen)
		{
			//DEBUG_LOG(("Using uncompressed data"));
			delete[] m_buffer;
			m_buffer = uncompBuffer;
			m_size = uncompLen;
		}
		else
		{
			//DEBUG_LOG(("Decompression failed - using compressed data"));
			// decompression failed.  Maybe we invalidly thought it was compressed?
			delete[] uncompBuffer;
		}
	}
	//if (m_size >= 4)
	//{
	//	DEBUG_LOG(("File starts as '%c%c%c%c'", m_buffer[0], m_buffer[1],
	//		m_buffer[2], m_buffer[3]));
	//}

	if (file)
	{
		file->close();
	}
	return m_size != 0;
}

void CachedFileInputStream::close(void)
{
	delete[] m_buffer;
	m_buffer=nullptr;

	m_pos=0;
	m_size=0;
}

Int CachedFileInputStream::read(void *pData, Int numBytes)
{
	if (m_buffer) {
		if ((numBytes+m_pos)>m_size) {
			numBytes=m_size-m_pos;
		}
		if (numBytes) {
			memcpy(pData,m_buffer+m_pos,numBytes);
			m_pos+=numBytes;
		}
		return(numBytes);
	}
	return 0;
}

UnsignedInt CachedFileInputStream::tell(void)
{
	return m_pos;
}

Bool CachedFileInputStream::absoluteSeek(UnsignedInt pos)
{
	if (pos<0) return false;
	if (pos>m_size) {
		pos=m_size;
	}
	m_pos=pos;
	return true;
}

Bool CachedFileInputStream::eof(void)
{
	return m_size==m_pos;
}

void CachedFileInputStream::rewind()
{
	m_pos=0;
}

// -----------------------------------------------------------

//
// FileInputStream - helper class.	Used to read in data using a FILE *
//
/*
FileInputStream::FileInputStream(void):m_file(nullptr)
{
}

FileInputStream::~FileInputStream(void)
{
	if (m_file != nullptr) {
		m_file->close();
		m_file = nullptr;
	}
}

Bool FileInputStream::open(AsciiString path)
{
	m_file = TheFileSystem->openFile(path.str(), File::READ | File::BINARY);
	return m_file == nullptr?false:true;
}

void FileInputStream::close(void)
{
	if (m_file != nullptr) {
		m_file->close();
		m_file = nullptr;
	}
}

Int FileInputStream::read(void *pData, Int numBytes)
{
	int bytesRead = 0;
	if (m_file != nullptr) {
		bytesRead = m_file->read(pData, numBytes);
	}
	return(bytesRead);
}

UnsignedInt FileInputStream::tell(void)
{
	UnsignedInt pos = 0;
	if (m_file != nullptr) {
		pos = m_file->position();
	}
	return(pos);
}

Bool FileInputStream::absoluteSeek(UnsignedInt pos)
{
	if (m_file != nullptr) {
		return (m_file->seek(pos, File::START) != -1);
	}
	return(false);
}

Bool FileInputStream::eof(void)
{
	if (m_file != nullptr) {
		return (m_file->size() == m_file->position());
	}
	return(true);
}

void FileInputStream::rewind()
{
	if (m_file != nullptr) {
		m_file->seek(0, File::START);
	}
}
*/

//----------------------------------------------------------------------
// DataChunkOutput
// Data will be stored to a temporary m_tmp_file until the DataChunkOutput
// object is destroyed.  At that time, the actual output m_tmp_file will
// be written, including a table of m_contents.
//----------------------------------------------------------------------

#define TEMP_FILENAME		"_tmpChunk.dat"

DataChunkOutput::DataChunkOutput( OutputStream *pOut ) :
m_pOut(pOut)
{
	AsciiString tmpFileName = TheGlobalData->getPath_UserData();
	tmpFileName.concat(TEMP_FILENAME);
	m_tmp_file = ::fopen( tmpFileName.str(), "wb" );
	m_chunkStack = nullptr;
}

DataChunkOutput::~DataChunkOutput()
{
	// store the table of m_contents
	m_contents.write(*m_pOut);

	// Rewind the temp m_tmp_file
	::fclose(m_tmp_file);

	AsciiString tmpFileName = TheGlobalData->getPath_UserData();
	tmpFileName.concat(TEMP_FILENAME);

 	m_tmp_file = ::fopen( tmpFileName.str(), "rb" );
	::fseek(m_tmp_file, 0, SEEK_SET);

	// append the temp m_tmp_file m_contents
	char buffer[256];
	int len = 256;
	while( len == 256 )
	{
		// copy data from the temp m_tmp_file to the output m_tmp_file
		len = ::fread( buffer, 1, 256, m_tmp_file );
		m_pOut->write( buffer, len );
	}

	::fclose(m_tmp_file);
}

void DataChunkOutput::openDataChunk( const char *name, DataChunkVersionType ver )
{
	// allocate (or get existing) ID from the table of m_contents
	UnsignedInt id = m_contents.allocateID( AsciiString(name) );

	// allocate a new chunk and place it on top of the chunk stack
	OutputChunk *c = newInstance(OutputChunk);
	c->next = m_chunkStack;
	m_chunkStack = c;
	m_chunkStack->id = id;

	// store the chunk ID
	::fwrite( (const char *)&id, sizeof(UnsignedInt), 1, m_tmp_file );

	// store the chunk version number
	::fwrite( (const char *)&ver, sizeof(DataChunkVersionType), 1, m_tmp_file );

	// remember this m_tmp_file position so we can write the real data size later
	c->filepos = ::ftell(m_tmp_file);
#ifdef VERBOSE
	DEBUG_LOG(("Writing chunk %s at %d (%x)", name, ::ftell(m_tmp_file), ::ftell(m_tmp_file)));
#endif
	// store a placeholder for the data size
	Int dummy = 0xffff;
	::fwrite( (const char *)&dummy, sizeof(Int), 1, m_tmp_file  );
}

void DataChunkOutput::closeDataChunk( void )
{
	if (m_chunkStack == nullptr)
	{
		// TODO: Throw exception
		return;
	}

	// remember where we are
	Int here = ::ftell(m_tmp_file);

	// rewind to store the data size
	::fseek(m_tmp_file, m_chunkStack->filepos , SEEK_SET);

	// compute data size (not including the actual data size itself)
	Int size = here - m_chunkStack->filepos - sizeof(Int);

	// store the data size
	::fwrite( (const char *)&size, sizeof(Int) , 1, m_tmp_file );

	// go back to where we were
	::fseek(m_tmp_file, here , SEEK_SET);

	// pop the chunk off the stack
	OutputChunk *c = m_chunkStack;
#ifdef VERBOSE
	DEBUG_LOG(("Closing chunk %s at %d (%x)", m_contents.getName(c->id).str(), here, here));
#endif
	m_chunkStack = m_chunkStack->next;
	deleteInstance(c);
}

void DataChunkOutput::writeReal( Real r )
{
	::fwrite( (const char *)&r, sizeof(float) , 1, m_tmp_file  );
}

void DataChunkOutput::writeInt( Int i )
{
	::fwrite( (const char *)&i, sizeof(Int) , 1, m_tmp_file );
}

void DataChunkOutput::writeByte( Byte b )
{
	::fwrite( (const char *)&b, sizeof(Byte) , 1, m_tmp_file );
}

void DataChunkOutput::writeArrayOfBytes(char *ptr, Int len)
{
	::fwrite( (const char *)ptr, 1, len , m_tmp_file );
}

void DataChunkOutput::writeAsciiString( const AsciiString& theString )
{
	UnsignedShort len = theString.getLength();
	::fwrite( (const char *)&len, sizeof(UnsignedShort) , 1, m_tmp_file );
	::fwrite( theString.str(), len , 1, m_tmp_file );
}

void DataChunkOutput::writeUnicodeString( UnicodeString theString )
{
	UnsignedShort len = theString.getLength();
	::fwrite( (const char *)&len, sizeof(UnsignedShort) , 1, m_tmp_file );
	::fwrite( theString.str(), len*sizeof(WideChar) , 1, m_tmp_file );
}

void DataChunkOutput::writeNameKey( const NameKeyType key )
{
		AsciiString kname = TheNameKeyGenerator->keyToName(key);
		Int keyAndType = m_contents.allocateID(kname);
		keyAndType <<= 8;
		Dict::DataType t = Dict::DICT_ASCIISTRING;
		keyAndType |= (t & 0xff);
		writeInt(keyAndType);
}

void DataChunkOutput::writeDict( const Dict& d )
{
	UnsignedShort len = d.getPairCount();
	::fwrite( (const char *)&len, sizeof(UnsignedShort) , 1, m_tmp_file );
	for (int i = 0; i < len; i++)
	{
		NameKeyType k = d.getNthKey(i);
		AsciiString kname = TheNameKeyGenerator->keyToName(k);

		Int keyAndType = m_contents.allocateID(kname);
		keyAndType <<= 8;
		Dict::DataType t = d.getNthType(i);
		keyAndType |= (t & 0xff);
		writeInt(keyAndType);

		switch(t)
		{
			case Dict::DICT_BOOL:
				writeByte(d.getNthBool(i)?1:0);
				break;
			case Dict::DICT_INT:
				writeInt(d.getNthInt(i));
				break;
			case Dict::DICT_REAL:
				writeReal(d.getNthReal(i));
				break;
			case Dict::DICT_ASCIISTRING:
				writeAsciiString(d.getNthAsciiString(i));
				break;
			case Dict::DICT_UNICODESTRING:
				writeUnicodeString(d.getNthUnicodeString(i));
				break;
			default:
				DEBUG_CRASH(("impossible"));
				break;
		}
	}
}

//----------------------------------------------------------------------
// DataChunkTableOfContents
//----------------------------------------------------------------------

DataChunkTableOfContents::DataChunkTableOfContents( void ) :
m_list(nullptr),
m_nextID(1),
m_listLength(0),
m_headerOpened(false)
{
}

DataChunkTableOfContents::~DataChunkTableOfContents()
{
	Mapping *m, *next;

	// free all list elements
	for( m=m_list; m; m=next )
	{
		next = m->next;
		deleteInstance(m);
	}
}

// return mapping data
Mapping *DataChunkTableOfContents::findMapping( const AsciiString& name )
{
	Mapping *m;

	for( m=m_list; m; m=m->next )
		if (name == m->name )
			return m;

	return nullptr;
}

// convert name to integer identifier
UnsignedInt DataChunkTableOfContents::getID( const AsciiString& name )
{
	Mapping *m = findMapping( name );

	if (m)
		return m->id;

	DEBUG_CRASH(("name not found in DataChunkTableOfContents::getName for name %s",name.str()));
	return 0;
}

// convert integer identifier to name
AsciiString DataChunkTableOfContents::getName( UnsignedInt id )
{
	Mapping *m;

	for( m=m_list; m; m=m->next )
		if (m->id == id)
			return m->name;

	DEBUG_CRASH(("name not found in DataChunkTableOfContents::getName for id %d",id));
	return AsciiString::TheEmptyString;
}

// create new ID for given name or return existing mapping
UnsignedInt DataChunkTableOfContents::allocateID(const AsciiString& name )
{
	Mapping *m = findMapping( name );

	if (m)
		return m->id;
	else
	{
		// allocate new id mapping
		m = newInstance(Mapping);

		m->id = m_nextID++;
		m->name =  name ;

		// prepend to list
		m->next = m_list;
		m_list = m;

		m_listLength++;

		return m->id;
	}
}

// output the table of m_contents to a binary m_tmp_file stream
void DataChunkTableOfContents::write( OutputStream &s )
{
	Mapping *m;
	unsigned char len;

	Byte tag[4]={'C','k', 'M', 'p'};	// Chunky height map. jba.
	s.write(tag,sizeof(tag));

	// output number of elements in the table
	s.write( (void *)&this->m_listLength, sizeof(Int) );

	// output symbol table
	for( m=this->m_list; m; m=m->next )
	{
		len = m->name.getLength();
		s.write( (char *)&len, sizeof(unsigned char) );
		s.write( (char *)m->name.str(),  len);
		s.write( (char *)&m->id, sizeof(UnsignedInt) );
	}
}

// read the table of m_contents from a binary m_tmp_file stream
// TODO: Should this reset the symbol table?
// Append symbols to table
void DataChunkTableOfContents::read( ChunkInputStream &s)
{
	Int count, i;
	UnsignedInt maxID = 0;
	unsigned char len;
	Mapping *m;

	Byte tag[4]={'x','x', 'x', 'x'};	// Chunky height map. jba.
	s.read(tag,sizeof(tag));
	if (tag[0] != 'C' || tag[1] != 'k' || tag[2] != 'M' || tag[3] != 'p') {
		return;	 // Don't throw, may happen with legacy files.
	}

	// get number of symbols in table
	s.read( (char *)&count, sizeof(Int) );

	for( i=0; i<count; i++ )
	{
		// allocate new id mapping
		m = newInstance(Mapping);

		// read string length
		s.read( (char *)&len, sizeof(unsigned char) );

		// allocate and read in string
		if (len>0) {
			char *str = m->name.getBufferForRead(len);
			s.read( str, len );
			str[len] = '\000';
		}

		// read id
		s.read( (char *)&m->id, sizeof(UnsignedInt) );

		// prepend to list
		m->next = this->m_list;
		this->m_list = m;

		this->m_listLength++;

		// track max ID used
		if (m->id > maxID)
			maxID = m->id;
	}
	m_headerOpened = count > 0 && !s.eof();

	// adjust next ID so no ID's are reused
	this->m_nextID = max( this->m_nextID, maxID+1 );
}

//----------------------------------------------------------------------
// DataChunkInput
//----------------------------------------------------------------------
DataChunkInput::DataChunkInput( ChunkInputStream *pStream ) : m_file( pStream ),
																										m_userData(nullptr),
																										m_currentObject(nullptr),
																										m_chunkStack(nullptr),
																										m_parserList(nullptr)
{
	// read table of m_contents
	m_contents.read(*m_file);

	// store location of first data chunk
	m_fileposOfFirstChunk = m_file->tell();
}

DataChunkInput::~DataChunkInput()
{
	clearChunkStack();

	UserParser *p, *next;
	for (p=m_parserList; p; p=next) {
		next = p->next;
		deleteInstance(p);
	}

}

// register a user parsing function for a given DataChunk label
void DataChunkInput::registerParser( const AsciiString& label, const AsciiString& parentLabel,
																		 DataChunkParserPtr parser, void *userData )
{
	UserParser *p = newInstance(UserParser);

	p->label.set( label );
	p->parentLabel.set(parentLabel );
	p->parser = parser;
	p->userData = userData;

	// prepend parser to parser list
	p->next = m_parserList;
	m_parserList = p;
}

// parse the chunk stream using registered parsers
// it is assumed that the file position is at the start of a data chunk
// (it can be inside a parent chunk) when parse is called.
Bool DataChunkInput::parse( void *userData )
{
	AsciiString label;
	AsciiString parentLabel;
	DataChunkVersionType ver;
	UserParser *parser;
	Bool scopeOK;
	DataChunkInfo info;

	// If the header wasn't a chunk table of contents, we can't parse.
	if (!m_contents.isOpenedForRead()) {
		return false;
	}

	// if we are inside a data chunk right now, get its name
	if (m_chunkStack)
		parentLabel = m_contents.getName( m_chunkStack->id );

	while( atEndOfFile() == false )
	{
		if (m_chunkStack) { // If we are parsing chunks in a chunk, check current length.
			if (m_chunkStack->dataLeft < CHUNK_HEADER_BYTES) {
				DEBUG_ASSERTCRASH( m_chunkStack->dataLeft==0, ("Unexpected extra data in chunk."));
				break;
			}
		}
		// open the chunk
		label = openDataChunk( &ver );
		if (atEndOfFile()) { // FILE * returns eof after you read past end of file, so check.
			break;
		}

		// find a registered parser for this chunk
		for( parser=m_parserList; parser; parser=parser->next )
		{
			// chunk labels must match
			if ( parser->label == label )
			{
				// make sure parent name (scope) also matches
				scopeOK = true;

				if (parentLabel != parser->parentLabel)
					scopeOK = false;

				if (scopeOK)
				{
					// m_tmp_file out the chunk info and call the user parser
					info.label = label;
					info.parentLabel = parentLabel;
					info.version = ver;
					info.dataSize = getChunkDataSize();

					if (parser->parser( *this, &info, userData ) == false)
						return false;
					break;
				}
			}
		}

		// close chunk (and skip to end if need be)
		closeDataChunk();
	}

	return true;
}

// clear the stack
void DataChunkInput::clearChunkStack( void )
{
	InputChunk *c, *next;

	for( c=m_chunkStack; c; c=next )
	{
		next = c->next;
		deleteInstance(c);
	}

	m_chunkStack = nullptr;
}

// reset the stream to just-opened state - ready to parse the first chunk
void DataChunkInput::reset( void )
{
	clearChunkStack();
	m_file->absoluteSeek( m_fileposOfFirstChunk );
}

// Checks if the file has our initial tag word.
Bool DataChunkInput::isValidFileType(void)
{
	return m_contents.isOpenedForRead();
}

AsciiString DataChunkInput::openDataChunk(DataChunkVersionType *ver )
{
	// allocate a new chunk and place it on top of the chunk stack
	InputChunk *c = newInstance(InputChunk);
	c->id = 0;
	c->version = 0;
	c->dataSize = 0;
	//DEBUG_LOG(("Opening data chunk at offset %d (%x)", m_file->tell(), m_file->tell()));
	// read the chunk ID
	m_file->read( (char *)&c->id, sizeof(UnsignedInt) );
	decrementDataLeft( sizeof(UnsignedInt) );

	// read the chunk version number
	m_file->read( (char *)&c->version, sizeof(DataChunkVersionType) );
	decrementDataLeft( sizeof(DataChunkVersionType) );

	// read the chunk data size
	m_file->read( (char *)&c->dataSize, sizeof(Int) );
	decrementDataLeft( sizeof(Int) );

	// all of the data remains to be read
	c->dataLeft = c->dataSize;
	c->chunkStart = m_file->tell();

	*ver = c->version;

	c->next = m_chunkStack;
	m_chunkStack = c;
	if (this->atEndOfFile()) {
		return AsciiString::TheEmptyString;
	}
	return m_contents.getName( c->id );
}

// close chunk and move to start of next chunk
void DataChunkInput::closeDataChunk( void )
{
	if (m_chunkStack == nullptr)
	{
		// TODO: Throw exception
		return;
	}

	if (m_chunkStack->dataLeft > 0)
	{
		// skip past the remainder of this chunk
		m_file->absoluteSeek( m_file->tell()+m_chunkStack->dataLeft );
		decrementDataLeft( m_chunkStack->dataLeft );

	}

	// pop the chunk off the stack
	InputChunk *c = m_chunkStack;
	m_chunkStack = m_chunkStack->next;
	deleteInstance(c);
}


// return label of current data chunk
AsciiString DataChunkInput::getChunkLabel( void )
{
	if (m_chunkStack == nullptr)
	{
		// TODO: Throw exception
		DEBUG_CRASH(("Bad."));
		return AsciiString::TheEmptyString;
	}

	return m_contents.getName( m_chunkStack->id );
}

// return version of current data chunk
DataChunkVersionType DataChunkInput::getChunkVersion( void )
{
	if (m_chunkStack == nullptr)
	{
		// TODO: Throw exception
		DEBUG_CRASH(("Bad."));
		return 0;
	}

	return m_chunkStack->version;
}

// return size of data stored in this chunk
UnsignedInt DataChunkInput::getChunkDataSize( void )
{
	if (m_chunkStack == nullptr)
	{
		// TODO: Throw exception
		DEBUG_CRASH(("Bad."));
		return 0;
	}

	return m_chunkStack->dataSize;
}


// return size of data left to read in this chunk
UnsignedInt DataChunkInput::getChunkDataSizeLeft( void )
{
	if (m_chunkStack == nullptr)
	{
		// TODO: Throw exception
		DEBUG_CRASH(("Bad."));
		return 0;
	}

	return m_chunkStack->dataLeft;
}

Bool DataChunkInput::atEndOfChunk( void )
{
	if (m_chunkStack)
	{
		if (m_chunkStack->dataLeft <= 0)
			return true;
		return false;
	}

	return true;
}

// update data left in chunk(s)
// since data read from a chunk is also read from all parent chunks,
// traverse the chunk stack and decrement the data left for each
void DataChunkInput::decrementDataLeft( Int size )
{
	InputChunk *c;

	c = m_chunkStack;
	while (c) {
		c->dataLeft -= size;
		c = c->next;
	}
	// The sizes of the parent chunks on the stack are adjusted in closeDataChunk.
}

Real DataChunkInput::readReal(void)
{
	Real r;
	DEBUG_ASSERTCRASH(m_chunkStack->dataLeft>=sizeof(Real), ("Read past end of chunk."));
	m_file->read( (char *)&r, sizeof(Real) );
	decrementDataLeft( sizeof(Real) );
	return r;
}

Int DataChunkInput::readInt(void)
{
	Int i;
	DEBUG_ASSERTCRASH(m_chunkStack->dataLeft>=sizeof(Int), ("Read past end of chunk."));
	m_file->read( (char *)&i, sizeof(Int) );
	decrementDataLeft( sizeof(Int) );
	return i;
}

Byte DataChunkInput::readByte(void)
{
	Byte b;
	DEBUG_ASSERTCRASH(m_chunkStack->dataLeft>=sizeof(Byte), ("Read past end of chunk."));
	m_file->read( (char *)&b, sizeof(Byte) );
	decrementDataLeft( sizeof(Byte) );
	return b;
}

void DataChunkInput::readArrayOfBytes(char *ptr, Int len)
{
	DEBUG_ASSERTCRASH(m_chunkStack->dataLeft>=len, ("Read past end of chunk."));
	m_file->read( ptr, len );
	decrementDataLeft( len );
}

NameKeyType DataChunkInput::readNameKey(void)
{
		Int keyAndType = readInt();
#ifdef DEBUG_CRASHING
		Dict::DataType t = (Dict::DataType)(keyAndType & 0xff);
		DEBUG_ASSERTCRASH(t==Dict::DICT_ASCIISTRING,("Invalid key data."));
#endif
		keyAndType >>= 8;

		AsciiString kname = m_contents.getName(keyAndType);
		NameKeyType k = TheNameKeyGenerator->nameToKey(kname);
		return k;
}

Dict DataChunkInput::readDict()
{
	UnsignedShort len;
	DEBUG_ASSERTCRASH(m_chunkStack->dataLeft>=sizeof(UnsignedShort), ("Read past end of chunk."));
	m_file->read( &len, sizeof(UnsignedShort) );
	decrementDataLeft( sizeof(UnsignedShort) );
	DEBUG_ASSERTCRASH(m_chunkStack->dataLeft>=len, ("Read past end of chunk."));

	Dict d(len);

	for (int i = 0; i < len; i++)
	{
		Int keyAndType = readInt();
		Dict::DataType t = (Dict::DataType)(keyAndType & 0xff);
		keyAndType >>= 8;

		AsciiString kname = m_contents.getName(keyAndType);
		NameKeyType k = TheNameKeyGenerator->nameToKey(kname);

		switch(t)
		{
			case Dict::DICT_BOOL:
				d.setBool(k, readByte() ? true : false);
				break;
			case Dict::DICT_INT:
				d.setInt(k, readInt());
				break;
			case Dict::DICT_REAL:
				d.setReal(k, readReal());
				break;
			case Dict::DICT_ASCIISTRING:
				d.setAsciiString(k, readAsciiString());
				break;
			case Dict::DICT_UNICODESTRING:
				d.setUnicodeString(k, readUnicodeString());
				break;
			default:
				throw ERROR_CORRUPT_FILE_FORMAT;
				break;
		}
	}

	return d;
}

AsciiString DataChunkInput::readAsciiString(void)
{
	UnsignedShort len;
	DEBUG_ASSERTCRASH(m_chunkStack->dataLeft>=sizeof(UnsignedShort), ("Read past end of chunk."));
	m_file->read( &len, sizeof(UnsignedShort) );
	decrementDataLeft( sizeof(UnsignedShort) );
	DEBUG_ASSERTCRASH(m_chunkStack->dataLeft>=len, ("Read past end of chunk."));
	AsciiString theString;
	if (len>0) {
		char *str = theString.getBufferForRead(len);
		m_file->read( str, len );
		decrementDataLeft( len );
		// add null delimiter to string.  Note that getBufferForRead allocates space for terminating null.
		str[len] = '\000';
	}

	return theString;
}

UnicodeString DataChunkInput::readUnicodeString(void)
{
	UnsignedShort len;
	DEBUG_ASSERTCRASH(m_chunkStack->dataLeft>=sizeof(UnsignedShort), ("Read past end of chunk."));
	m_file->read( &len, sizeof(UnsignedShort) );
	decrementDataLeft( sizeof(UnsignedShort) );
	DEBUG_ASSERTCRASH(m_chunkStack->dataLeft>=len, ("Read past end of chunk."));
	UnicodeString theString;
	if (len>0) {
		WideChar *str = theString.getBufferForRead(len);
		m_file->read( (char*)str, len*sizeof(WideChar) );
		decrementDataLeft( len*sizeof(WideChar) );
		// add null delimiter to string.  Note that getBufferForRead allocates space for terminating null.
		str[len] = '\000';
	}

	return theString;
}
