#include "stdafx.h"
#include "sql.h"

/*SDOC***********************************************************************
	
	Name:			Lexer::Lexer
						Lexer::~Lexer

	Action:		Constructor/Destructor

***********************************************************************EDOC*/
Lexer::Lexer(QIODevice& file) 
	: file(file), 
		YYCURSOR(0), 
		YYLIMIT(0), 
		state(0), 
		line(1) 
{
}

Lexer::~Lexer() 
{
}


/*SDOC***********************************************************************
	
	Name:			Lexer::YYFILL

	Action:		Fills the internal data buffer with data read from the file,
						and resets the cursor pointer.

***********************************************************************EDOC*/
void Lexer::YYFILL(int n) 
{
	static int oldline = -1;

	char buffer[4096];
	qint64 bytesRead;
	if((bytesRead = file.read(buffer, sizeof(buffer))) > 0)
	{
		QByteArray newData(YYCURSOR, YYLIMIT-YYCURSOR);
		newData.append(buffer, bytesRead);
		data = newData;

		if(oldline != line) 
		{
			cout << "line " << line << endl;
			oldline = line;
		}

		YYCURSOR = data.begin();
		YYLIMIT = data.end();
	}
	else
	{
		// Make we pad with enough NULLs to prevent any buffer overflows;
		// eventually we'll scan the NULL terminator and exit.
		/*!max:re2c*/
		for(int i = 0; i < YYMAXFILL; ++i) data.append((char)0);
		#undef YYMAXFILL
	}
}


/*SDOC***********************************************************************
	
	Name:			Lexer::scan

	Action:		Performs the lexical scan and returns the next token.

	Returns:	EOF_SYMBOL		- on end of file/input
						ERROR_STATE		- on unrecognized input
						T_*						- recognized token identifier

***********************************************************************EDOC*/
int Lexer::scan()
{
	#define BEGIN_STATE(theState)	case theState: {
	#define END_STATE() } break;
	enum state_t { DEFAULT, LINECOMMENT, COMMENT, STRING };

	QByteArray::const_iterator YYMARKER = YYCURSOR;
	QByteArray::const_iterator YYTOKEN = YYCURSOR;
	/*!re2c
	re2c:define:YYCTYPE = "unsigned char";
	re2c:indent:top			= 2;
	any	= [\000-\377];
	*/

	while(1) switch(state) 
	{
	case DEFAULT: /*!re2c
			[\000]									{ return SqlParser::EOF_SYMBOL; }
			"--"										{ state = LINECOMMENT; continue; }
			"/*"										{ state = COMMENT; continue; }
			[']											{ state = STRING; continue; }	
			[ \t\v\f]+							{ continue; }
			[\n]										{ ++line; continue; }
			[;]											{ return SqlParser::T_SEMICOLON; }
			[(]											{ return SqlParser::T_LPAREN; }
			[)]											{ return SqlParser::T_RPAREN; }
			[,]											{ return SqlParser::T_COMMA; }
			[=]											{ return SqlParser::T_EQUALS; }
			"CREATE"								{ return SqlParser::T_CREATE; }
			"DATABASE"							{ return SqlParser::T_DATABASE; }
			"USE"										{ return SqlParser::T_USE; }
			"DROP"									{ return SqlParser::T_DROP; }
			"IF"										{ return SqlParser::T_IF; }
			"EXISTS"								{ return SqlParser::T_EXISTS; }
			"TABLE"									{ return SqlParser::T_TABLE; }
			"NOT"										{ return SqlParser::T_NOT; }
			"NULL"									{ return SqlParser::T_NULL; }
			"PRIMARY"								{ return SqlParser::T_PRIMARY; }
			"KEY"										{ return SqlParser::T_KEY; }
			"auto_increment"				{ return SqlParser::T_AUTOINCREMENT; }
			"default"								{ return SqlParser::T_DEFAULT; }
			"LOCK TABLES"						{ return SqlParser::T_LOCK; }
			"UNLOCK TABLES"					{ return SqlParser::T_UNLOCK; }
			"WRITE"									{ return SqlParser::T_WRITE; }
			"INSERT INTO" 					{ return SqlParser::T_INSERTINTO; }
			"VALUES"								{ return SqlParser::T_VALUES; }
			"COMMENT"								{ return SqlParser::T_COMMENT; }
			"DELIMITER"							{ return SqlParser::T_DELIMITER; }
			"ENGINE=MyISAM"					{ continue; }
			"AUTO_INCREMENT="[0-9]+ { continue; }
			"DEFAULT CHARSET=utf8"	{ continue; }
			[`][a-zA-Z_][a-zA-Z0-9_]*[`]	
															{ tokenData = QString(QByteArray::fromRawData(YYTOKEN+1, YYCURSOR-YYTOKEN-2)); return SqlParser::T_QNAME; }
			"int"|"varchar"|"tinyint"|"double"|"date"|"time"|"mediumtext"|"longtext"		
															{ tokenData = QString(QByteArray::fromRawData(YYTOKEN, YYCURSOR-YYTOKEN-1)); return SqlParser::T_DATATYPE; }
			[0-9]*[\.][0-9]+				{ tokenData = QString(QByteArray::fromRawData(YYTOKEN, YYCURSOR-YYTOKEN-1)); return SqlParser::T_DOUBLE; }
			[0-9]+									{ tokenData = QString(QByteArray::fromRawData(YYTOKEN, YYCURSOR-YYTOKEN-1)); return SqlParser::T_INTEGER; }
			any											{ tokenData = QString(QByteArray::fromRawData(YYTOKEN, min(40, YYLIMIT-YYTOKEN))); return SqlParser::ERROR_STATE; } */
	case LINECOMMENT: /*!re2c
			[\000]									{ return SqlParser::EOF_SYMBOL; }
			[\n]										{ ++line; state = DEFAULT; continue; }
			any											{ continue; } */
	case COMMENT: /*!re2c
			[\000]									{ return SqlParser::EOF_SYMBOL; }
			"*" "/"									{ state = DEFAULT; continue; }
			[\n]										{ ++line; continue; }
			any											{ continue; } */
	case STRING: /*!re2c
			[\000]									{ return SqlParser::EOF_SYMBOL; }
			[']											{ state = DEFAULT; return SqlParser::T_STRING; }
			[\\][']									{ tokenData = tokenData.toString().append("''"); continue; }
			[\\][t]									{ tokenData = tokenData.toString().append("\t"); continue; }
			[\\][r]									{ tokenData = tokenData.toString().append("\r"); continue; }
			[\\][n]									{ tokenData = tokenData.toString().append("\n"); continue; }
			[\\] .									{ tokenData = tokenData.toString().append(*(YYCURSOR-1)); continue; }
			any											{ tokenData = tokenData.toString().append(*(YYCURSOR-1)); continue; } */
	}
	return SqlParser::ERROR_STATE;
}
