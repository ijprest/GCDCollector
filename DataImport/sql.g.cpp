%parser         SqlParser
%decl           sql.h
%impl           sql.cpp

%expect 0

%token ERROR_STATE
%token T_WS							"_ws"
%token T_SEMICOLON			";"
%token T_LPAREN					"("
%token T_RPAREN					")"
%token T_COMMA					","
%token T_EQUALS					"="
%token T_CREATE					"CREATE"
%token T_DATABASE				"DATABASE"
%token T_QNAME					"qualified_name"
%token T_USE						"USE"
%token T_DROP						"DROP"
%token T_IF							"IF"
%token T_EXISTS					"EXISTS"
%token T_TABLE					"TABLE"
%token T_DATATYPE				"data_type"
%token T_INTEGER				"integer_literal"
%token T_DOUBLE					"real_literal"
%token T_STRING					"string_literal"
%token T_NOT						"NOT"
%token T_NULL						"NULL"
%token T_AUTOINCREMENT	"auto_increment"
%token T_DEFAULT				"default"
%token T_PRIMARY				"PRIMARY"
%token T_KEY						"KEY"
%token T_WRITE					"WRITE"
%token T_LOCK						"LOCK TABLES"
%token T_UNLOCK					"UNLOCK TABLES"
%token T_INSERTINTO			"INSERT INTO"
%token T_VALUES					"VALUES"
%token T_COMMENT				"COMMENT"
%token T_DELIMITER			"DELIMITER"

%start Program

/:
#ifndef SQLPARSER_H
#define SQLPARSER_H

#include "sqlparser_p.h"

class Lexer
{
public:
	Lexer(QIODevice& file);
	~Lexer();

	int scan();
	QVariant tokenSym() { return tokenData; }
	int lineNumber() { return line; }

private:
	QVariant tokenData;
	QIODevice& file;
	QByteArray data;
	int state;
	int line;
	QByteArray::const_iterator YYCURSOR;
	QByteArray::const_iterator YYLIMIT;
	void YYFILL(int n);
};

class Parser : protected $table
{
public:
	Parser(QIODevice& file) : lexer(file) {}
  ~Parser() {}

  bool parse();

  int errorLineNumber() const { return m_errorLineNumber; }
  QString errorMessage() const { return m_errorMessage; }
  QVariant result() const { return m_result; }

private:
  inline QVariant& sym(int index) { return (m_stateStack.end()+index-1)->second; }

	Lexer lexer;
	typedef std::pair<int, QVariant> state_t;
	QStack<state_t> m_stateStack;
  int m_errorLineNumber;
  QString m_errorMessage;
  QVariant m_result;
};

#endif // SQLPARSER_H
:/

/.
#include "stdafx.h"
#include "sql.h"

bool Parser::parse()
{
  int yytoken = -1;

  m_stateStack.push(state_t(0,QVariant()));
  while(1) 
  {
    const int state = m_stateStack.top().first;
    if(yytoken == -1 && -TERMINAL_COUNT != action_index[state])
      yytoken = lexer.scan();

    int act = t_action(state, yytoken);
    if(act == ACCEPT_STATE)
      return true;
    else if(act > 0) 
    {
      m_stateStack.push(state_t(act,QVariant()));
      yytoken = -1;
    } 
    else if(act < 0) 
    {
      int reduce = - act - 1;

			for(int i = 0; i < rhs[reduce]; ++i) 
				m_stateStack.pop();
      act = m_stateStack.top().first;

      switch(reduce) 
      {
./

Program: StatementList ;
StatementList: Statement ;
StatementList: StatementList Statement ;
Statement: T_SEMICOLON ;
Statement: T_DELIMITER T_SEMICOLON ;
Statement: T_CREATE T_DATABASE T_QNAME T_SEMICOLON ;
Statement: T_USE T_QNAME T_SEMICOLON ;
Statement: T_DROP T_TABLE T_IF T_EXISTS T_QNAME T_SEMICOLON ;
Statement: T_CREATE T_TABLE T_QNAME T_LPAREN ColumnDefList T_RPAREN T_SEMICOLON ;
ColumnDefList: ColumnDef ;
ColumnDefList: ColumnDefList T_COMMA ColumnDef ;
ColumnDef: T_QNAME T_DATATYPE ColumnSize ColumnNotNull ColumnAuto ColumnDefault ColumnComment ;
ColumnSize: ;
ColumnSize: T_LPAREN T_INTEGER T_RPAREN ;
ColumnNotNull: ;
ColumnNotNull: T_NOT T_NULL ;
ColumnAuto: ;
ColumnAuto: T_AUTOINCREMENT ;
ColumnComment: ;
ColumnComment: T_COMMENT T_STRING ;
ColumnDefault: ;
ColumnDefault: T_DEFAULT Value ;
ColumnDef: T_KEY T_QNAME KeyDef ;
ColumnDef: T_PRIMARY T_KEY KeyDef ;
KeyDef: T_LPAREN T_QNAME T_RPAREN ;
KeyDef: T_LPAREN T_QNAME T_LPAREN T_INTEGER T_RPAREN T_RPAREN ;
Statement: T_LOCK T_QNAME T_WRITE T_SEMICOLON ;
Statement: T_UNLOCK T_SEMICOLON ;
Statement: T_INSERTINTO T_QNAME T_VALUES MultiInsertValues T_SEMICOLON ;
MultiInsertValues: T_LPAREN ValueList T_RPAREN ;
MultiInsertValues: MultiInsertValues T_COMMA T_LPAREN ValueList T_RPAREN;
ValueList: Value ;
ValueList: ValueList T_COMMA Value ;
Value: T_INTEGER ;
Value: T_STRING ;
Value: T_NULL ;

/.
case $rule_number: {
    //sym(1) = lexer.tokenSym();
} break;
./

/.
			} // switch

			m_stateStack.push( state_t(nt_action(act, lhs[reduce] - TERMINAL_COUNT),QVariant()) );
		} 
		else 
		{
			int ers = state;
			int shifts = 0;
			int reduces = 0;
			int expected_tokens[3];
			for(int tk = 0; tk < TERMINAL_COUNT; ++tk) 
			{
				int k = t_action(ers, tk);
				if(!k)
					continue;
				else if(k < 0)
					++reduces;
				else if(spell[tk]) 
				{
					if(shifts < 3)
						expected_tokens[shifts] = tk;
					++shifts;
				}
			}

			m_errorLineNumber = lexer.lineNumber();
			m_errorMessage.clear();
			if(shifts && shifts < 3) 
			{
				bool first = true;

				for(int s = 0; s < shifts; ++s) 
				{
					if(first)
						m_errorMessage += QLatin1String("Expected ");
					else
						m_errorMessage += QLatin1String(", ");

					first = false;
					m_errorMessage += QLatin1String("`");
					m_errorMessage += QLatin1String(spell[expected_tokens[s]]);
					m_errorMessage += QLatin1String("'");
				}
				if(yytoken >= 0) 
				{
					m_errorMessage += QLatin1String(", got ");
					m_errorMessage += QLatin1String(spell[yytoken]);
				}
			}
			return false;
		}
	}
	return false;
}

./
