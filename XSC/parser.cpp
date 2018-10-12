/*

    Project.

        XSC - The XtremeScript Compiler Version 0.8

    Abstract.

        Parser module

    Date Created.

        8.21.2002

    Author.

        Alex Varanese

*/

// ---- Include Files -------------------------------------------------------------------------

    #include "parser.h"
    #include "error.h"
    #include "lexer.h"
    #include "symbol_table.h"
    #include "func_table.h"
    #include "i_code.h"

// ---- Globals -------------------------------------------------------------------------------

    // ---- Functions -------------------------------------------------------------------------

        int g_iCurrScope;                               // The current scope

    // ---- Loops -----------------------------------------------------------------------------

        Stack g_LoopStack;                              // Loop handling stack

// ---- Functions -----------------------------------------------------------------------------

    /******************************************************************************************
    *
    *   ReadToken ()
    *
    *   Attempts to read a specific token and prints an error if its not found.
    */

    void ReadToken ( Token ReqToken )
    {
        // Determine if the next token is the required one

        if ( GetNextToken () != ReqToken )
        {
            // If not, exit on a specific error

            char pstrErrorMssg [ 256 ];
            switch ( ReqToken )
            {
                // Integer

                case TOKEN_TYPE_INT:
                    strcpy ( pstrErrorMssg, "Integer" );
                    break;

                // Float

                case TOKEN_TYPE_FLOAT:
                    strcpy ( pstrErrorMssg, "Float" );
                    break;

                // Identifier

                case TOKEN_TYPE_IDENT:
                    strcpy ( pstrErrorMssg, "Identifier" );
                    break;

                // var

                case TOKEN_TYPE_RSRVD_VAR:
                    strcpy ( pstrErrorMssg, "var" );
                    break;

                // true

                case TOKEN_TYPE_RSRVD_TRUE:
                    strcpy ( pstrErrorMssg, "true" );
                    break;

                // false

                case TOKEN_TYPE_RSRVD_FALSE:
                    strcpy ( pstrErrorMssg, "false" );
                    break;

                // if

                case TOKEN_TYPE_RSRVD_IF:
                    strcpy ( pstrErrorMssg, "if" );
                    break;

                // else

                case TOKEN_TYPE_RSRVD_ELSE:
                    strcpy ( pstrErrorMssg, "else" );
                    break;

                // break

                case TOKEN_TYPE_RSRVD_BREAK:
                    strcpy ( pstrErrorMssg, "break" );
                    break;

                // continue

                case TOKEN_TYPE_RSRVD_CONTINUE:
                    strcpy ( pstrErrorMssg, "continue" );
                    break;

                // for

                case TOKEN_TYPE_RSRVD_FOR:
                    strcpy ( pstrErrorMssg, "for" );
                    break;

                // while

                case TOKEN_TYPE_RSRVD_WHILE:
                    strcpy ( pstrErrorMssg, "while" );
                    break;

                // func

                case TOKEN_TYPE_RSRVD_FUNC:
                    strcpy ( pstrErrorMssg, "func" );
                    break;

                // return

                case TOKEN_TYPE_RSRVD_RETURN:
                    strcpy ( pstrErrorMssg, "return" );
                    break;

                // host

                case TOKEN_TYPE_RSRVD_HOST:
                    strcpy ( pstrErrorMssg, "host" );
                    break;

                // Operator

                case TOKEN_TYPE_OP:
                    strcpy ( pstrErrorMssg, "Operator" );
                    break;

                // Comma

                case TOKEN_TYPE_DELIM_COMMA:
                    strcpy ( pstrErrorMssg, "," );
                    break;

                // Open parenthesis

                case TOKEN_TYPE_DELIM_OPEN_PAREN:
                    strcpy ( pstrErrorMssg, "(" );
                    break;

                // Close parenthesis

                case TOKEN_TYPE_DELIM_CLOSE_PAREN:
                    strcpy ( pstrErrorMssg, ")" );
                    break;

                // Open brace

                case TOKEN_TYPE_DELIM_OPEN_BRACE:
                    strcpy ( pstrErrorMssg, "[" );
                    break;

                // Close brace

                case TOKEN_TYPE_DELIM_CLOSE_BRACE:
                    strcpy ( pstrErrorMssg, "]" );
                    break;

                // Open curly brace

                case TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE:
                    strcpy ( pstrErrorMssg, "{" );
                    break;

                // Close curly brace

                case TOKEN_TYPE_DELIM_CLOSE_CURLY_BRACE:
                    strcpy ( pstrErrorMssg, "}" );
                    break;

                // Semicolon

                case TOKEN_TYPE_DELIM_SEMICOLON:
                    strcpy ( pstrErrorMssg, ";" );
                    break;

                // String

                case TOKEN_TYPE_STRING:
                    strcpy ( pstrErrorMssg, "String" );
                    break;
            }

            // Finish the message

            strcat ( pstrErrorMssg, " expected" );

            // Display the error

            ExitOnCodeError ( pstrErrorMssg );
        }
    }

    /******************************************************************************************
    *
    *   IsOpRelational ()
    *
    *   Determines if the specified operator is a relational operator.
    */

    int IsOpRelational ( int iOpType )
    {
        if ( iOpType != OP_TYPE_EQUAL &&
             iOpType != OP_TYPE_NOT_EQUAL &&
             iOpType != OP_TYPE_LESS &&
             iOpType != OP_TYPE_GREATER &&
             iOpType != OP_TYPE_LESS_EQUAL &&
             iOpType != OP_TYPE_GREATER_EQUAL )
            return FALSE;
        else
            return TRUE;
    }

    /******************************************************************************************
    *
    *   IsOpLogical ()
    *
    *   Determines if the specified operator is a logical operator.
    */

    int IsOpLogical ( int iOpType )
    {
        if ( iOpType != OP_TYPE_LOGICAL_AND &&
             iOpType != OP_TYPE_LOGICAL_OR &&
             iOpType != OP_TYPE_LOGICAL_NOT )
            return FALSE;
        else
            return TRUE;
    }

    /******************************************************************************************
    *
    *   ParseSourceCode ()
    *
    *   Parses the source code from start to finish, generating a complete I-code
    *   representation.
    *   从头到尾解析源码文件，生成完成的中间代码表示。
    */

    void ParseSourceCode ()
    {
        // Reset the lexer
        // 重置词法解析器

        ResetLexer ();

        // Initialize the loop stack
        // 初始化循环stack，用于生成循环代码

        InitStack ( & g_LoopStack );

        // Set the current scope to global
        // 设置当前的scope为全局

        g_iCurrScope = SCOPE_GLOBAL;

        // Parse each line of code
        // 解析每一行代码

        while ( TRUE )
        {
            // Parse the next statement and ignore an end of file marker
            // 解析下一个语句并且忽略文件结束标记

            ParseStatement ();

            // If we're at the end of the token stream, break the parsing loop
            // 如果我们到达了token流的末尾，就跳出循环

            if ( GetNextToken () == TOKEN_TYPE_END_OF_STREAM )
                break;
            else
                RewindTokenStream ();
        }

        // Free the loop stack
        // 释放loop循环栈

        FreeStack ( & g_LoopStack );
    }

    /******************************************************************************************
    *
    *   ParseStatement ()
    *
    *   Parses a statement.
    */

    void ParseStatement ()
    {
        // If the next token is a semicolon, the statement is empty so return
        // 如果下一个token是分号，直接返回

        if ( GetLookAheadChar () == ';' )
        {
            ReadToken ( TOKEN_TYPE_DELIM_SEMICOLON );
            return;
        }

        // Determine the initial token of the statement
        // 判定语句的初始化token

        Token InitToken = GetNextToken ();

        // Branch to a parse function based on the token
        // 根据语句的初始化token类型匹配对应解析函数

        switch ( InitToken )
        {
            // Unexpected end of file
            // 未期待的文件末尾

            case TOKEN_TYPE_END_OF_STREAM:
                ExitOnCodeError ( "Unexpected end of file" );
                break;

            // Block
            // 代码块

            case TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE:
                ParseBlock ();
                break;

            // Variable/array declaration
            // 变量、数组声明

            case TOKEN_TYPE_RSRVD_VAR:
                ParseVar ();
                break;

            // Host API function import
            // 内置API函数导入

            case TOKEN_TYPE_RSRVD_HOST:
                ParseHost ();
                break;

            // Function definition
            // 函数定义

            case TOKEN_TYPE_RSRVD_FUNC:
                ParseFunc ();
                break;

            // if block
            // if语句

            case TOKEN_TYPE_RSRVD_IF:
                ParseIf ();
                break;

            // while loop block
            // while语句

            case TOKEN_TYPE_RSRVD_WHILE:
                ParseWhile ();
                break;

            // for loop block
            // for循环语句

            case TOKEN_TYPE_RSRVD_FOR:
                ParseFor ();
                break;

            // break
            // break语句

            case TOKEN_TYPE_RSRVD_BREAK:
                ParseBreak ();
                break;

            // continue
            // continue语句

            case TOKEN_TYPE_RSRVD_CONTINUE:
                ParseContinue ();
                break;

            // return
            // return语句

            case TOKEN_TYPE_RSRVD_RETURN:
                ParseReturn ();
                break;

            // Assignment or Function Call
            // 赋值或者函数调用

            case TOKEN_TYPE_IDENT:
            {
                // What kind of identifier is it?
                // 查看变量是何种类型

                if ( GetSymbolByIdent ( GetCurrLexeme (), g_iCurrScope ) )
                {
                    // It's an identifier, so treat the statement as an assignment
                    // 如果是一个变量，则按照赋值对待

                    ParseAssign ();
                }
                else if ( GetFuncByName ( GetCurrLexeme () ) )
                {
                    // It's a function

                    // Annotate the line and parse the call
                    // 是函数调用就注释此行并解析调用

                    AddICodeSourceLine ( g_iCurrScope, GetCurrSourceLine () );
                    ParseFuncCall ();

                    // Verify the presence of the semicolon

                    ReadToken ( TOKEN_TYPE_DELIM_SEMICOLON );
                }
                else
                {
                    // It's invalid

                    ExitOnCodeError ( "Invalid identifier" );
                }

                break;
            }

            // Anything else is invalid

            default:
                ExitOnCodeError ( "Unexpected input" );
                break;
        }
    }

    /******************************************************************************************
    *
    *   ParseBlock ()
    *
    *   Parses a code block.
    *
    *       { <Statement-List> }
    *   解析代码块
    *   代码块是以大括号包围的语句列表
    */

    void ParseBlock ()
    {
        // Make sure we're not in the global scope
        // 确保当前解析的位置不是在全局范围中
        // 不支持在全局范围定义代码块

        if ( g_iCurrScope == SCOPE_GLOBAL )
            ExitOnCodeError ( "Code blocks illegal in global scope" );

        // Read each statement until the end of the block
        // 读取并解析每个单独的语句，直到代码块末尾

        while ( GetLookAheadChar () != '}' )
            ParseStatement ();

        // Read the closing curly brace
        // 读取右大括号

        ReadToken ( TOKEN_TYPE_DELIM_CLOSE_CURLY_BRACE );
    }

    /******************************************************************************************
    *
    *   ParseVar ()
    *
    *   Parses the declaration of a variable or array and adds it to the symbol table.
    *
    *       var <Identifier>;
    *       var <Identifier> [ <Integer> ];
    *   解析变量声明或者数组声明，并将它添加到符号表中
    */

    void ParseVar ()
    {
        // Read an identifier token
        // 读取一个标识符token

        ReadToken ( TOKEN_TYPE_IDENT );

        // Copy the current lexeme into a local string buffer to save the variable's identifier
        // 复制当前Identifier到local字符串缓冲区

        char pstrIdent [ MAX_LEXEME_SIZE ];
        CopyCurrLexeme ( pstrIdent );

        // Set the size to 1 for a variable (an array will update this value)
        // 设置变量的大小为1（数组会更改这个值）

        int iSize = 1;

        // Is the look-ahead character an open brace?
        // 如果向前看一个字符为中括号[

        if ( GetLookAheadChar () == '[' )
        {
            // Verify the open brace
            // 验证左中括号

            ReadToken ( TOKEN_TYPE_DELIM_OPEN_BRACE );

            // If so, read an integer token
            // 如果没问题则读取一个整数token

            ReadToken ( TOKEN_TYPE_INT );

            // Convert the current lexeme to an integer to get the size
            // 将当前lexeme转换为一个整数以获取size

            iSize = atoi ( GetCurrLexeme () );

            // Read the closing brace
            // 读取右中括号

            ReadToken ( TOKEN_TYPE_DELIM_CLOSE_BRACE );
        }

        // Add the identifier and size to the symbol table
        // 将标识符和size添加添加到符号表

        if ( AddSymbol ( pstrIdent, iSize, g_iCurrScope, SYMBOL_TYPE_VAR ) == -1 )
            ExitOnCodeError ( "Identifier redefinition" );

        // Read the semicolon
        // 读取分号

        ReadToken ( TOKEN_TYPE_DELIM_SEMICOLON );
    }

    /******************************************************************************************
    *
    *   ParseHostAPIFuncImport ()
    *
    *   Parses the importing of a host API function.
    *
    *       host <Identifier> ();
    *   解析导入的内置函数
    */

    void ParseHost ()
    {
        // Read the host API function name
        // 读取内置函数名

        ReadToken ( TOKEN_TYPE_IDENT );

        // Add the function to the function table with the host API flag set
        // 将函数添加到函数表并设置内置函数的FLAG

        if ( AddFunc ( GetCurrLexeme (), TRUE ) == -1 )
            ExitOnCodeError ( "Function redefinition" );

        // Make sure the function name is followed with ()
        // 确保函数名后面是一对小括号

        ReadToken ( TOKEN_TYPE_DELIM_OPEN_PAREN );
        ReadToken ( TOKEN_TYPE_DELIM_CLOSE_PAREN );

        // Read the semicolon
        // 读取分号

        ReadToken ( TOKEN_TYPE_DELIM_SEMICOLON );
    }

    /******************************************************************************************
    *
    *   ParseFunc ()
    *
    *   Parses a function.
    *
    *       func <Identifier> ( <Parameter-List> ) <Statement>
    *       func <标识符>   (参数列表) <语句>
    *
    *   解析函数
    */

    void ParseFunc ()
    {
        // Make sure we're not already in a function
        // 确保当前解析的位置不是在函数中
        // 因为不支持函数嵌套

        if ( g_iCurrScope != SCOPE_GLOBAL )
            ExitOnCodeError ( "Nested functions illegal" );

        // Read the function name
        // 读取函数名

        ReadToken ( TOKEN_TYPE_IDENT );

        // Add the non-host API function to the function table and get its index
        // 将非host函数添加到函数表并获取它的index

        int iFuncIndex = AddFunc ( GetCurrLexeme (), FALSE );

        // Check for a function redefinition
        // 根据索引检查是否发生函数重复定义

        if ( iFuncIndex == -1 )
            ExitOnCodeError ( "Function redefinition" );

        // Set the scope to the function
        // 将当前解析位置设置为函数在函数表中的index

        g_iCurrScope = iFuncIndex;

        // Read the opening parenthesis
        // 读取左圆括号

        ReadToken ( TOKEN_TYPE_DELIM_OPEN_PAREN );

        // Use the look-ahead character to determine if the function takes parameters
        // 向前看一个字符检查函数是否有参数

        if ( GetLookAheadChar () != ')' )
        {
            // If the function being defined is _Main (), flag an error since _Main ()
            // cannot accept paraemters
            // 如果这个函数被定义个_Main()，则报错，因为_Main()函数没有参数

            /*
            if ( g_ScriptHeader.iIsMainFuncPresent &&
                 g_ScriptHeader.iMainFuncIndex == iFuncIndex )
            {
                ExitOnCodeError ( "_Main () cannot accept parameters" );
            }
             */

            // Start the parameter count at zero
            // 参数数量计数器

            int iParamCount = 0;

            // Crete an array to store the parameter list locally
            // 创建一个数组保存参数列表

            char ppstrParamList [ MAX_FUNC_DECLARE_PARAM_COUNT ][ MAX_IDENT_SIZE ];

            // Read the parameters
            // 读取参数

            while ( TRUE )
            {
                // Read the identifier
                // 读取标识符

                ReadToken ( TOKEN_TYPE_IDENT );

                // Copy the current lexeme to the parameter list array
                // 复制当前lexeme到参数列表数组

                CopyCurrLexeme ( ppstrParamList [ iParamCount ] );

                // Increment the parameter count
                // 参数计数器自增

                ++ iParamCount;

                // Check again for the closing parenthesis to see if the parameter list is done
                // 检查右小括号确认参数列表是否结束

                if ( GetLookAheadChar () == ')' )
                    break;

                // Otherwise read a comma and move to the next parameter
                // 读取参数之间的逗号分隔

                ReadToken ( TOKEN_TYPE_DELIM_COMMA );
            }

            // Set the final parameter count
            // 设置最终的参数计数

            SetFuncParamCount ( g_iCurrScope, iParamCount );

            // Write the parameters to the function's symbol table in reverse order, so they'll
            // be emitted from right-to-left
            // 用相反的顺序将参数写入到函数的符号表，这样它们在生成时就使用从右到左的顺序

            while ( iParamCount > 0 )
            {
                -- iParamCount;

                // Add the parameter to the symbol table
                // 添加参数到符号表

                AddSymbol ( ppstrParamList [ iParamCount ], 1, g_iCurrScope, SYMBOL_TYPE_PARAM );
            }
        }

        // Read the closing parenthesis
        // 读取右圆括号

        ReadToken ( TOKEN_TYPE_DELIM_CLOSE_PAREN );

        // Read the opening curly brace
        // 读取左大括号

        ReadToken ( TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE );

        // Parse the function's body
        // 解析函数体代码

        ParseBlock ();

        // Return to the global scope
        // 解析函数完毕，设置scope为global

        g_iCurrScope = SCOPE_GLOBAL;
    }

    /******************************************************************************************
    *
    *   ParseExpr ()
    *
    *   Parses an expression.
    *   解析表达式
    */

    void ParseExpr ()
    {
        int iInstrIndex;

        // The current operator type
        // 当前操作符类型

        int iOpType;

        // Parse the subexpression
        // 解析子表达式

        ParseSubExpr ();

        // Parse any subsequent relational or logical operators
        // 解析关系和逻辑运算符

        while ( TRUE )
        {
            // Get the next token

            if ( GetNextToken () != TOKEN_TYPE_OP ||
                 ( ! IsOpRelational ( GetCurrOp () ) &&
                   ! IsOpLogical ( GetCurrOp () ) ) )
            {
                RewindTokenStream ();
                break;
            }

            // Save the operator
            // 保存操作符

            iOpType = GetCurrOp ();

            // Parse the second term
            // 解析第二个term

            ParseSubExpr ();

            // Pop the first operand into _T1
            // 将第一个操作数弹出_T1

            iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_POP );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex );

            // Pop the second operand into _T0
            // 将第二个操作数弹出到_T0

            iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_POP );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );

            // ---- Perform the binary operation associated with the specified operator
            // ---- 准备与特定操作数关联的二进制操作

            // Determine the operator type
            // 检测操作符类型

            if ( IsOpRelational ( iOpType ) )
            {
                // Get a pair of free jump target indices
                // 获取一对自由跳转的索引

                int iTrueJumpTargetIndex = GetNextJumpTargetIndex (),
                    iExitJumpTargetIndex = GetNextJumpTargetIndex ();

                // It's a relational operator
                // 是关系操作符

                switch ( iOpType )
                {
                    // Equal
                    // 等于

                    case OP_TYPE_EQUAL:
                    {
                        // Generate a JE instruction
                        // 生成JE指令

                        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_JE );
                        break;
                    }

                    // Not Equal
                    // 不等于

                    case OP_TYPE_NOT_EQUAL:
                    {
                        // Generate a JNE instruction
                        // 生成JNE指令

                        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_JNE );
                        break;
                    }

                    // Greater
                    // 大于

                    case OP_TYPE_GREATER:
                    {
                        // Generate a JG instruction
                        // 生成JG指令

                        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_JG );
                        break;
                    }

                    // Less
                    // 小于

                    case OP_TYPE_LESS:
                    {
                        // Generate a JL instruction
                        // 生成JL指令

                        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_JL );
                        break;
                    }

                    // Greater or Equal
                    // 大于等于

                    case OP_TYPE_GREATER_EQUAL:
                    {
                        // Generate a JGE instruction
                        // 生成JGE指令

                        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_JGE );
                        break;
                    }

                    // Less Than or Equal
                    // 小于等于

                    case OP_TYPE_LESS_EQUAL:
                    {
                        // Generate a JLE instruction
                        // 生成JLE指令

                        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_JLE );
                        break;
                    }
                }

                // Add the jump instruction's operands (_T0 and _T1)
                // 增加跳转指令的操作数 (_T0 和 _T1)

                AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );
                AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex );

                // 生成跳转的指令索引 
                // 这个索引紧跟着操作数，例如： JG _T0, _T1, _L1
                AddJumpTargetICodeOp ( g_iCurrScope, iInstrIndex, iTrueJumpTargetIndex );

                // Generate the outcome for falsehood
                // 跳转指令为结果为false，则将0这个数PUSH到栈上
                // PUSH 0

                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_PUSH );
                AddIntICodeOp ( g_iCurrScope, iInstrIndex, 0 );

                // Generate a jump past the true outcome
                // 生成一个跳转指令以跳过true
                // true就是下面向栈顶放数字1
                // 例如JMP _L2

                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_JMP );
                AddJumpTargetICodeOp ( g_iCurrScope, iInstrIndex, iExitJumpTargetIndex );

                // Set the jump target for the true outcome
                // 为true结果设置跳转的target

                AddICodeJumpTarget ( g_iCurrScope, iTrueJumpTargetIndex );

                // Generate the outcome for truth
                // 跳转指令为结果为true，则将1这个数PUSH到栈上

                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_PUSH );
                AddIntICodeOp ( g_iCurrScope, iInstrIndex, 1 );

                // Set the jump target for exiting the operand evaluation
                // 为操作数的求值设置跳转目标
                // 即当前操作已经结束，结束前设置跳转索引

                AddICodeJumpTarget ( g_iCurrScope, iExitJumpTargetIndex );
            }
            else
            {
                // It must be a logical operator

                switch ( iOpType )
                {
                    // And

                    case OP_TYPE_LOGICAL_AND:
                    {
                        // Get a pair of free jump target indices

                        int iFalseJumpTargetIndex = GetNextJumpTargetIndex (),
                            iExitJumpTargetIndex = GetNextJumpTargetIndex ();

                        // JE _T0, 0, True

                        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_JE );
                        AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );
                        AddIntICodeOp ( g_iCurrScope, iInstrIndex, 0 );
                        AddJumpTargetICodeOp ( g_iCurrScope, iInstrIndex, iFalseJumpTargetIndex );

                        // JE _T1, 0, True

                        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_JE );
                        AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex );
                        AddIntICodeOp ( g_iCurrScope, iInstrIndex, 0 );
                        AddJumpTargetICodeOp ( g_iCurrScope, iInstrIndex, iFalseJumpTargetIndex );

                        // Push 1

                        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_PUSH );
                        AddIntICodeOp ( g_iCurrScope, iInstrIndex, 1 );

                        // Jmp Exit

                        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_JMP );
                        AddJumpTargetICodeOp ( g_iCurrScope, iInstrIndex, iExitJumpTargetIndex );

                        // L0: (False)

                        AddICodeJumpTarget ( g_iCurrScope, iFalseJumpTargetIndex );

                        // Push 0

                        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_PUSH );
                        AddIntICodeOp ( g_iCurrScope, iInstrIndex, 0 );

                        // L1: (Exit)

                        AddICodeJumpTarget ( g_iCurrScope, iExitJumpTargetIndex );

                        break;
                    }

                    // Or

                    case OP_TYPE_LOGICAL_OR:
                    {
                        // Get a pair of free jump target indices

                        int iTrueJumpTargetIndex = GetNextJumpTargetIndex (),
                            iExitJumpTargetIndex = GetNextJumpTargetIndex ();

                        // JNE _T0, 0, True

                        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_JNE );
                        AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );
                        AddIntICodeOp ( g_iCurrScope, iInstrIndex, 0 );
                        AddJumpTargetICodeOp ( g_iCurrScope, iInstrIndex, iTrueJumpTargetIndex );

                        // JNE _T1, 0, True

                        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_JNE );
                        AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex );
                        AddIntICodeOp ( g_iCurrScope, iInstrIndex, 0 );
                        AddJumpTargetICodeOp ( g_iCurrScope, iInstrIndex, iTrueJumpTargetIndex );

                        // Push 0

                        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_PUSH );
                        AddIntICodeOp ( g_iCurrScope, iInstrIndex, 0 );

                        // Jmp Exit

                        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_JMP );
                        AddJumpTargetICodeOp ( g_iCurrScope, iInstrIndex, iExitJumpTargetIndex );

                        // L0: (True)

                        AddICodeJumpTarget ( g_iCurrScope, iTrueJumpTargetIndex );

                        // Push 1

                        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_PUSH );
                        AddIntICodeOp ( g_iCurrScope, iInstrIndex, 1 );

                        // L1: (Exit)

                        AddICodeJumpTarget ( g_iCurrScope, iExitJumpTargetIndex );

                        break;
                    }
                }
            }
        }
    }

    /******************************************************************************************
    *
    *   ParseSubExpr ()
    *
    *   Parses a sub expression.
    */

    void ParseSubExpr ()
    {
        int iInstrIndex;

        // The current operator type

        int iOpType;

        // Parse the first term

        ParseTerm ();

        // Parse any subsequent +, - or $ operators

        while ( TRUE )
        {
            // Get the next token

            if ( GetNextToken () != TOKEN_TYPE_OP ||
                 ( GetCurrOp () != OP_TYPE_ADD &&
                   GetCurrOp () != OP_TYPE_SUB &&
                   GetCurrOp () != OP_TYPE_CONCAT ) )
            {
                RewindTokenStream ();
                break;
            }

            // Save the operator

            iOpType = GetCurrOp ();

            // Parse the second term

            ParseTerm ();

            // Pop the first operand into _T1

            iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_POP );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex );

            // Pop the second operand into _T0

            iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_POP );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );

            // Perform the binary operation associated with the specified operator

            int iOpInstr;
            switch ( iOpType )
            {
                // Binary addition

                case OP_TYPE_ADD:
                    iOpInstr = INSTR_ADD;
                    break;

                // Binary subtraction

                case OP_TYPE_SUB:
                    iOpInstr = INSTR_SUB;
                    break;

                // Binary string concatenation

                case OP_TYPE_CONCAT:
                    iOpInstr = INSTR_CONCAT;
                    break;
            }
            iInstrIndex = AddICodeInstr ( g_iCurrScope, iOpInstr );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex );

            // Push the result (stored in _T0)

            iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_PUSH );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );
        }
    }

    /******************************************************************************************
    *
    *   ParseTerm ()
    *
    *   Parses a term.
    */

    void ParseTerm ()
    {
        int iInstrIndex;

        // The current operator type

        int iOpType;

        // Parse the first factor

        ParseFactor ();

        // Parse any subsequent *, /, %, ^, &, |, #, << and >> operators

        while ( TRUE )
        {
            // Get the next token

            if ( GetNextToken () != TOKEN_TYPE_OP ||
                 ( GetCurrOp () != OP_TYPE_MUL &&
                   GetCurrOp () != OP_TYPE_DIV &&
                   GetCurrOp () != OP_TYPE_MOD &&
                   GetCurrOp () != OP_TYPE_EXP &&
                   GetCurrOp () != OP_TYPE_BITWISE_AND &&
                   GetCurrOp () != OP_TYPE_BITWISE_OR &&
                   GetCurrOp () != OP_TYPE_BITWISE_XOR &&
                   GetCurrOp () != OP_TYPE_BITWISE_SHIFT_LEFT &&
                   GetCurrOp () != OP_TYPE_BITWISE_SHIFT_RIGHT ) )
            {
                RewindTokenStream ();
                break;
            }

            // Save the operator

            iOpType = GetCurrOp ();

            // Parse the second factor

            ParseFactor ();

            // Pop the first operand into _T1

            iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_POP );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex );

            // Pop the second operand into _T0

            iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_POP );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );

            // Perform the binary operation associated with the specified operator

            int iOpInstr;
            switch ( iOpType )
            {
                // Binary multiplication

                case OP_TYPE_MUL:
                    iOpInstr = INSTR_MUL;
                    break;

                // Binary division

                case OP_TYPE_DIV:
                    iOpInstr = INSTR_DIV;
                    break;

                // Binary modulus

                case OP_TYPE_MOD:
                    iOpInstr = INSTR_MOD;
                    break;

                // Binary exponentiation

                case OP_TYPE_EXP:
                    iOpInstr = INSTR_EXP;
                    break;

                // Binary bitwise AND

                case OP_TYPE_BITWISE_AND:
                    iOpInstr = INSTR_AND;
                    break;

                // Binary bitwise OR

                case OP_TYPE_BITWISE_OR:
                    iOpInstr = INSTR_OR;
                    break;

                // Binary bitwise XOR

                case OP_TYPE_BITWISE_XOR:
                    iOpInstr = INSTR_XOR;
                    break;

                // Binary bitwise shift left

                case OP_TYPE_BITWISE_SHIFT_LEFT:
                    iOpInstr = INSTR_SHL;
                    break;

                // Binary bitwise shift left

                case OP_TYPE_BITWISE_SHIFT_RIGHT:
                    iOpInstr = INSTR_SHR;
                    break;
            }
            iInstrIndex = AddICodeInstr ( g_iCurrScope, iOpInstr );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex );

            // Push the result (stored in _T0)

            iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_PUSH );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );
        }
    }

    /******************************************************************************************
    *
    *   ParseFactor ()
    *
    *   Parses a factor.
    */

    void ParseFactor ()
    {
        int iInstrIndex;
        int iUnaryOpPending = FALSE;
        int iOpType;

        // First check for a unary operator
        // 首先检查单目运算符

        if ( GetNextToken () == TOKEN_TYPE_OP &&
             ( GetCurrOp () == OP_TYPE_ADD ||
               GetCurrOp () == OP_TYPE_SUB ||
               GetCurrOp () == OP_TYPE_BITWISE_NOT ||
               GetCurrOp () == OP_TYPE_LOGICAL_NOT ) )
        {
            // If it was found, save it and set the unary operator flag
            // 如果发现了，就将它保存到变量中

            iUnaryOpPending = TRUE;
            iOpType = GetCurrOp ();
        }
        else
        {
            // Otherwise rewind the token stream

            RewindTokenStream ();
        }

        // Determine which type of factor we're dealing with based on the next token

        switch ( GetNextToken () )
        {
            // It's a true or false constant, so push either 0 and 1 onto the stack

            case TOKEN_TYPE_RSRVD_TRUE:
            case TOKEN_TYPE_RSRVD_FALSE:
                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_PUSH );
                AddIntICodeOp ( g_iCurrScope, iInstrIndex, GetCurrToken () == TOKEN_TYPE_RSRVD_TRUE ? 1 : 0 );
                break;

            // It's an integer literal, so push it onto the stack

            case TOKEN_TYPE_INT:
                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_PUSH );
                AddIntICodeOp ( g_iCurrScope, iInstrIndex, atoi ( GetCurrLexeme () ) );
                break;

            // It's a float literal, so push it onto the stack

            case TOKEN_TYPE_FLOAT:
                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_PUSH );
                AddFloatICodeOp ( g_iCurrScope, iInstrIndex, ( float ) atof ( GetCurrLexeme () ) );
                break;

            // It's a string literal, so add it to the string table and push the resulting
            // string index onto the stack

            case TOKEN_TYPE_STRING:
            {
                int iStringIndex = AddString ( & g_StringTable, GetCurrLexeme () );
                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_PUSH );
                AddStringICodeOp ( g_iCurrScope, iInstrIndex, iStringIndex );
                break;
            }

            // It's an identifier

            case TOKEN_TYPE_IDENT:
            {
                // First find out if the identifier is a variable or array

                SymbolNode * pSymbol = GetSymbolByIdent ( GetCurrLexeme (), g_iCurrScope );
                if ( pSymbol )
                {
                    // Does an array index follow the identifier?

                    if ( GetLookAheadChar () == '[' )
                    {
                        // Ensure the variable is an array

                        if ( pSymbol->iSize == 1 )
                            ExitOnCodeError ( "Invalid array" );

                        // Verify the opening brace

                        ReadToken ( TOKEN_TYPE_DELIM_OPEN_BRACE );

                        // Make sure an expression is present

                        if ( GetLookAheadChar () == ']' )
                            ExitOnCodeError ( "Invalid expression" );

                        // Parse the index as an expression recursively

                        ParseExpr ();

                        // Make sure the index is closed

                        ReadToken ( TOKEN_TYPE_DELIM_CLOSE_BRACE );

                        // Pop the resulting value into _T0 and use it as the index variable

                        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_POP );
                        AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );

                        // Push the original identifier onto the stack as an array, indexed
                        // with _T0

                        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_PUSH );
                        AddArrayIndexVarICodeOp ( g_iCurrScope, iInstrIndex, pSymbol->iIndex, g_iTempVar0SymbolIndex );
                    }
                    else
                    {
                        // If not, make sure the identifier is not an array, and push it onto
                        // the stack

                        if ( pSymbol->iSize == 1 )
                        {
                            iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_PUSH );
                            AddVarICodeOp ( g_iCurrScope, iInstrIndex, pSymbol->iIndex );
                        }
                        else
                        {
                            ExitOnCodeError ( "Arrays must be indexed" );
                        }
                    }
                }
                else
                {
                    // The identifier wasn't a variable or array, so find out if it's a
                    // function

                    if ( GetFuncByName ( GetCurrLexeme () ) )
                    {
                        // It is, so parse the call

                        ParseFuncCall ();

                        // Push the return value

                        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_PUSH );
                        AddRegICodeOp ( g_iCurrScope, iInstrIndex, REG_CODE_RETVAL );
                    }
                }

                break;
            }

            // It's a nested expression, so call ParseExpr () recursively and validate the
            // presence of the closing parenthesis

            case TOKEN_TYPE_DELIM_OPEN_PAREN:
                ParseExpr ();
                ReadToken ( TOKEN_TYPE_DELIM_CLOSE_PAREN );
                break;

            // Anything else is invalid

            default:
                ExitOnCodeError ( "Invalid input" );
        }

        // Is a unary operator pending?

        if ( iUnaryOpPending )
        {
            // If so, pop the result of the factor off the top of the stack

            iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_POP );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );

            // Perform the unary operation

            if ( iOpType == OP_TYPE_LOGICAL_NOT )
            {
                // Get a pair of free jump target indices

                int iTrueJumpTargetIndex = GetNextJumpTargetIndex (),
                    iExitJumpTargetIndex = GetNextJumpTargetIndex ();

                // JE _T0, 0, True

                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_JE );
                AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );
                AddIntICodeOp ( g_iCurrScope, iInstrIndex, 0 );
                AddJumpTargetICodeOp ( g_iCurrScope, iInstrIndex, iTrueJumpTargetIndex );

                // Push 0

                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_PUSH );
                AddIntICodeOp ( g_iCurrScope, iInstrIndex, 0 );

                // Jmp L1

                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_JMP );
                AddJumpTargetICodeOp ( g_iCurrScope, iInstrIndex, iExitJumpTargetIndex );

                // L0: (True)

                AddICodeJumpTarget ( g_iCurrScope, iTrueJumpTargetIndex );

                // Push 1

                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_PUSH );
                AddIntICodeOp ( g_iCurrScope, iInstrIndex, 1 );

                // L1: (Exit)

                AddICodeJumpTarget ( g_iCurrScope, iExitJumpTargetIndex );
            }
            else
            {
                int iOpIndex;
                switch ( iOpType )
                {
                    // Negation

                    case OP_TYPE_SUB:
                        iOpIndex = INSTR_NEG;
                        break;

                    // Bitwise not

                    case OP_TYPE_BITWISE_NOT:
                        iOpIndex = INSTR_NOT;
                        break;
                }

                // Add the instruction's operand

                iInstrIndex = AddICodeInstr ( g_iCurrScope, iOpIndex );
                AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );

                // Push the result onto the stack

                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_PUSH );
                AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );
            }
        }
    }

    /******************************************************************************************
    *
    *   ParseIf ()
    *
    *   Parses an if block.
    *   解析if代码块
    *
    *       if ( <Expression> ) <Statement>
    *       if ( <表达式> ) <语句>
    *       if ( <Expression> ) <Statement> else <Statement>
    *       if ( <表达式> ) <语句> else <语句>
    */

    void ParseIf ()
    {
        int iInstrIndex;

        // Make sure we're inside a function
        // 确保当前在函数内部

        if ( g_iCurrScope == SCOPE_GLOBAL )
            ExitOnCodeError ( "if illegal in global scope" );

        // Annotate the line
        // 注释当前行

        AddICodeSourceLine ( g_iCurrScope, GetCurrSourceLine () );

        // Create a jump target to mark the beginning of the false block
        // 创建一个跳转目标(自增数字)用于标记false块的开头

        int iFalseJumpTargetIndex = GetNextJumpTargetIndex ();

        // Read the opening parenthesis
        // 读取左圆括号

        ReadToken ( TOKEN_TYPE_DELIM_OPEN_PAREN );

        // Parse the expression and leave the result on the stack
        // 解析表达式并且将其结果放在栈上

        ParseExpr ();

        // Read the closing parenthesis
        // 读取右圆括号

        ReadToken ( TOKEN_TYPE_DELIM_CLOSE_PAREN );

        // Pop the result into _T0 and compare it to zero
        // 将if括号中表达式的值的结果弹出到_TO，并且将结果和0对比

        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_POP );
        AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );

        // If the result is zero, jump to the false target

        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_JE );
        AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );
        AddIntICodeOp ( g_iCurrScope, iInstrIndex, 0 );
        AddJumpTargetICodeOp ( g_iCurrScope, iInstrIndex, iFalseJumpTargetIndex );

        // Parse the true block

        ParseStatement ();

        // Look for an else clause

        if ( GetNextToken () == TOKEN_TYPE_RSRVD_ELSE )
        {
            // If it's found, append the true block with an unconditional jump past the false
            // block

            int iSkipFalseJumpTargetIndex = GetNextJumpTargetIndex ();
            iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_JMP );
            AddJumpTargetICodeOp ( g_iCurrScope, iInstrIndex, iSkipFalseJumpTargetIndex );

            // Place the false target just before the false block

            AddICodeJumpTarget ( g_iCurrScope, iFalseJumpTargetIndex );

            // Parse the false block

            ParseStatement ();

            // Set a jump target beyond the false block

            AddICodeJumpTarget ( g_iCurrScope, iSkipFalseJumpTargetIndex );
        }
        else
        {
            // Otherwise, put the token back

            RewindTokenStream ();

            // Place the false target after the true block

            AddICodeJumpTarget ( g_iCurrScope, iFalseJumpTargetIndex );
        }
    }

    /******************************************************************************************
    *
    *   ParseWhile ()
    *
    *   Parses a while loop block.
    *
    *       while ( <Expression> ) <Statement>
    */

    void ParseWhile ()
    {
        int iInstrIndex;

        // Make sure we're inside a function

        if ( g_iCurrScope == SCOPE_GLOBAL )
            ExitOnCodeError ( "Statement illegal in global scope" );

        // Annotate the line

        AddICodeSourceLine ( g_iCurrScope, GetCurrSourceLine () );

        // Get two jump targets; for the top and bottom of the loop

        int iStartTargetIndex = GetNextJumpTargetIndex (),
            iEndTargetIndex = GetNextJumpTargetIndex ();

        // Set a jump target at the top of the loop

        AddICodeJumpTarget ( g_iCurrScope, iStartTargetIndex );

        // Read the opening parenthesis

        ReadToken ( TOKEN_TYPE_DELIM_OPEN_PAREN );

        // Parse the expression and leave the result on the stack

        ParseExpr ();

        // Read the closing parenthesis

        ReadToken ( TOKEN_TYPE_DELIM_CLOSE_PAREN );

        // Pop the result into _T0 and jump out of the loop if it's nonzero

        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_POP );
        AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );

        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_JE );
        AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );
        AddIntICodeOp ( g_iCurrScope, iInstrIndex, 0 );
        AddJumpTargetICodeOp ( g_iCurrScope, iInstrIndex, iEndTargetIndex );

        // Create a new loop instance structure

        Loop * pLoop = ( Loop * ) malloc ( sizeof ( Loop ) );

        // Set the starting and ending jump target indices

        pLoop->iStartTargetIndex = iStartTargetIndex;
        pLoop->iEndTargetIndex = iEndTargetIndex;

        // Push the loop structure onto the stack

        Push ( & g_LoopStack, pLoop );

        // Parse the loop body

        ParseStatement ();

        // Pop the loop instance off the stack

        Pop ( & g_LoopStack );

        // Unconditionally jump back to the start of the loop

        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_JMP );
        AddJumpTargetICodeOp ( g_iCurrScope, iInstrIndex, iStartTargetIndex );

        // Set a jump target for the end of the loop

        AddICodeJumpTarget ( g_iCurrScope, iEndTargetIndex );
    }

    /******************************************************************************************
    *
    *   ParseFor ()
    *
    *   Parses a for loop block.
    *
    *       for ( <Initializer>; <Condition>; <Perpetuator> ) <Statement>
    */

    void ParseFor ()
    {
        if ( g_iCurrScope == SCOPE_GLOBAL )
            ExitOnCodeError ( "for illegal in global scope" );

        // Annotate the line

        AddICodeSourceLine ( g_iCurrScope, GetCurrSourceLine () );

        /*
            A for loop parser implementation could go here
        */
    }

    /******************************************************************************************
    *
    *   ParseBreak ()
    *
    *   Parses a break statement.
    */

    void ParseBreak ()
    {
        // Make sure we're in a loop

        if ( IsStackEmpty ( & g_LoopStack ) )
            ExitOnCodeError ( "break illegal outside loops" );

        // Annotate the line

        AddICodeSourceLine ( g_iCurrScope, GetCurrSourceLine () );

        // Attempt to read the semicolon

        ReadToken ( TOKEN_TYPE_DELIM_SEMICOLON );

        // Get the jump target index for the end of the loop

        int iTargetIndex = ( ( Loop * ) Peek ( & g_LoopStack ) )->iEndTargetIndex;

        // Unconditionally jump to the end of the loop

        int iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_JMP );
        AddJumpTargetICodeOp ( g_iCurrScope, iInstrIndex, iTargetIndex );
    }

    /******************************************************************************************
    *
    *   ParseContinue ()
    *
    *   Parses a continue statement.
    */

    void ParseContinue ()
    {
        // Make sure we're inside a function

        if ( IsStackEmpty ( & g_LoopStack ) )
            ExitOnCodeError ( "continue illegal outside loops" );

        // Annotate the line

        AddICodeSourceLine ( g_iCurrScope, GetCurrSourceLine () );

        // Attempt to read the semicolon

        ReadToken ( TOKEN_TYPE_DELIM_SEMICOLON );

        // Get the jump target index for the start of the loop

        int iTargetIndex = ( ( Loop * ) Peek ( & g_LoopStack ) )->iStartTargetIndex;

        // Unconditionally jump to the end of the loop

        int iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_JMP );
        AddJumpTargetICodeOp ( g_iCurrScope, iInstrIndex, iTargetIndex );
    }

    /******************************************************************************************
    *
    *   ParseReturn ()
    *
    *   Parses a return statement.
    *
    *   return;
    *   return <expr>;
    */

    void ParseReturn ()
    {
        int iInstrIndex;

        // Make sure we're inside a function

        if ( g_iCurrScope == SCOPE_GLOBAL )
            ExitOnCodeError ( "return illegal in global scope" );

        // Annotate the line

        AddICodeSourceLine ( g_iCurrScope, GetCurrSourceLine () );

        // If a semicolon doesn't appear to follow, parse the expression and place it in
        // _RetVal

        if ( GetLookAheadChar () != ';' )
        {
            // Parse the expression to calculate the return value and leave the result on the stack.

            ParseExpr ();

            // Determine which function we're returning from

            if ( g_ScriptHeader.iIsMainFuncPresent &&
                 g_ScriptHeader.iMainFuncIndex == g_iCurrScope )
            {
                // It is _Main (), so pop the result into _T0

                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_POP );
                AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );
            }
            else
            {
                // It's not _Main, so pop the result into the _RetVal register

                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_POP );
                AddRegICodeOp ( g_iCurrScope, iInstrIndex, REG_CODE_RETVAL );
            }
        }
        else
        {
            // Clear _T0 in case we're exiting _Main ()

            if ( g_ScriptHeader.iIsMainFuncPresent &&
                 g_ScriptHeader.iMainFuncIndex == g_iCurrScope )
            {

                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_MOV );
                AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );
                AddIntICodeOp ( g_iCurrScope, iInstrIndex, 0 );
            }
        }

        if ( g_ScriptHeader.iIsMainFuncPresent &&
             g_ScriptHeader.iMainFuncIndex == g_iCurrScope )
        {
            // It's _Main, so exit the script with _T0 as the exit code

            iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_EXIT );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );
        }
        else
        {
            // It's not _Main, so return from the function

            AddICodeInstr ( g_iCurrScope, INSTR_RET );
        }

		// Validate the presence of the semicolon

		ReadToken ( TOKEN_TYPE_DELIM_SEMICOLON );
    }

    /******************************************************************************************
    *
    *   ParseAssign ()
    *
    *   Parses an assignment statement.
    *
    *   <Ident> <Assign-Op> <Expr>;
    *
    *   解析赋值语句
    *   1. 从符号表中查找变量
    *   2. 向前看一个字符是否为数组变量，是则分析索引表达式
    *   3. 获取操作符，加减乘除等
    *   4. 解析操作符右边的表达式
    *   5. 将表达式的值弹出到_T0寄存器
    *   6. 生成操作符指令和其目的操作数
    *   7. 生成源操作数
    */

    void ParseAssign ()
    {
        // Make sure we're inside a function
        // 确保我们在函数中
        // 不支持全局赋值操作

        if ( g_iCurrScope == SCOPE_GLOBAL )
            ExitOnCodeError ( "Assignment illegal in global scope" );

        int iInstrIndex;

        // Assignment operator
        // 赋值操作符

        int iAssignOp;

        // Annotate the line
        // 注释行号

        AddICodeSourceLine ( g_iCurrScope, GetCurrSourceLine () );

        // ---- Parse the variable or array
        // 解析变量或数组

        // 在符号表中查找之前加入的符号，也就是变量
        SymbolNode * pSymbol = GetSymbolByIdent ( GetCurrLexeme (), g_iCurrScope );

        // Does an array index follow the identifier?
        // 变量后有数组索引吗？

        // 如果是数组就解析其中的索引
        // 索引可以是一个表达式
        int iIsArray = FALSE;
        if ( GetLookAheadChar () == '[' )
        {
            // Ensure the variable is an array
            // 确保之前查找的变量存在
            // 否则判断为不是数组

            if ( pSymbol->iSize == 1 )
                ExitOnCodeError ( "Invalid array" );

            // Verify the opening brace
            // 读取并验证做中括号

            ReadToken ( TOKEN_TYPE_DELIM_OPEN_BRACE );

            // Make sure an expression is present
            // 确保中括号里面有表达式
            // 否则就报错

            if ( GetLookAheadChar () == ']' )
                ExitOnCodeError ( "Invalid expression" );

            // Parse the index as an expression
            // 把index当作表达式解析

            ParseExpr ();

            // Make sure the index is closed

            ReadToken ( TOKEN_TYPE_DELIM_CLOSE_BRACE );

            // Set the array flag

            iIsArray = TRUE;
        }
        else
        {
            // Make sure the variable isn't an array

            if ( pSymbol->iSize > 1 )
               ExitOnCodeError ( "Arrays must be indexed" );
        }

        // ---- Parse the assignment operator
        // 解析赋值操作符

        // 如果不是赋值类的操作符就退出
        if ( GetNextToken () != TOKEN_TYPE_OP &&
             ( GetCurrOp () != OP_TYPE_ASSIGN &&
               GetCurrOp () != OP_TYPE_ASSIGN_ADD &&
               GetCurrOp () != OP_TYPE_ASSIGN_SUB &&
               GetCurrOp () != OP_TYPE_ASSIGN_MUL &&
               GetCurrOp () != OP_TYPE_ASSIGN_DIV &&
               GetCurrOp () != OP_TYPE_ASSIGN_MOD &&
               GetCurrOp () != OP_TYPE_ASSIGN_EXP &&
               GetCurrOp () != OP_TYPE_ASSIGN_CONCAT &&
               GetCurrOp () != OP_TYPE_ASSIGN_AND &&
               GetCurrOp () != OP_TYPE_ASSIGN_OR &&
               GetCurrOp () != OP_TYPE_ASSIGN_XOR &&
               GetCurrOp () != OP_TYPE_ASSIGN_SHIFT_LEFT &&
               GetCurrOp () != OP_TYPE_ASSIGN_SHIFT_RIGHT ) )
             ExitOnCodeError ( "Illegal assignment operator" );
        else
            // 否则获取操作符
            iAssignOp = GetCurrOp ();

        // ---- Parse the value expression
        // 解析值表达式

        ParseExpr ();

        // Validate the presence of the semicolon
        // 读取并验证表达式最后的分号

        ReadToken ( TOKEN_TYPE_DELIM_SEMICOLON );

        // Pop the value into _T0
        // 将栈顶的值弹出到_T0

        iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_POP );
        AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );

        // If the variable was an array, pop the top of the stack into _T1 for use as the index
        // 如果变量是一个数组，则把栈上的值弹出到_T1
        // 这个栈上的值是之前解析数组索引时的值

        if ( iIsArray )
        {
            iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_POP );
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar1SymbolIndex );
        }

        // ---- Generate the I-code for the assignment instruction
        // 为赋值指令生成中间码

        switch ( iAssignOp )
        {
            // =

            case OP_TYPE_ASSIGN:
                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_MOV );
                break;

            // +=

            case OP_TYPE_ASSIGN_ADD:
                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_ADD );
                break;

            // -=

            case OP_TYPE_ASSIGN_SUB:
                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_SUB );
                break;

            // *=

            case OP_TYPE_ASSIGN_MUL:
                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_MUL );
                break;

            // /=

            case OP_TYPE_ASSIGN_DIV:
                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_DIV );
                break;

            // %=

            case OP_TYPE_ASSIGN_MOD:
                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_MOD );
                break;

            // ^=

            case OP_TYPE_ASSIGN_EXP:
                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_EXP );
                break;

            // $=

            case OP_TYPE_ASSIGN_CONCAT:
                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_CONCAT );
                break;

            // &=

            case OP_TYPE_ASSIGN_AND:
                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_AND );
                break;

            // |=

            case OP_TYPE_ASSIGN_OR:
                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_OR );
                break;

            // #=

            case OP_TYPE_ASSIGN_XOR:
                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_XOR );
                break;

            // <<=

            case OP_TYPE_ASSIGN_SHIFT_LEFT:
                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_SHL );
                break;

            // >>=

            case OP_TYPE_ASSIGN_SHIFT_RIGHT:
                iInstrIndex = AddICodeInstr ( g_iCurrScope, INSTR_SHR );
                break;
        }

        // Generate the destination operand
        // 生成目标操作数

        if ( iIsArray )
            AddArrayIndexVarICodeOp ( g_iCurrScope, iInstrIndex, pSymbol->iIndex, g_iTempVar1SymbolIndex );
        else
            AddVarICodeOp ( g_iCurrScope, iInstrIndex, pSymbol->iIndex );

        // Generate the source
        // 生成源操作数

        AddVarICodeOp ( g_iCurrScope, iInstrIndex, g_iTempVar0SymbolIndex );
    }

    /******************************************************************************************
    *
    *   ParseFuncCall ()
    *
    *   Parses a function call
    *   解析一个函数调用
    *
    *   <Ident> ( <Expr>, <Expr> );
    *   <表示符> ( <表达式>, <表达式> );
    */

    void ParseFuncCall ()
    {
        // Get the function by it's identifier
        // 通过标识符获取当前函数

        FuncNode * pFunc = GetFuncByName ( GetCurrLexeme () );

        // It is, so start the parameter count at zero
        // 从0开始计算参数的数量

        int iParamCount = 0;

        // Attempt to read the opening parenthesis
        // 读取左圆括号

        ReadToken ( TOKEN_TYPE_DELIM_OPEN_PAREN );

        // Parse each parameter and push it onto the stack
        // 解析每一个参数并把它PUSH到栈上

        while ( TRUE )
        {
            // Find out if there's another parameter to push
            // 检查是否还有其他参数需要PUSH

            if ( GetLookAheadChar () != ')' )
            {
                // There is, so parse it as an expression
                // 将参数当作表达式解析

                ParseExpr ();

                // Increment the parameter count and make sure it's not greater than the amount
                // accepted by the function (unless it's a host API function
                // 增加参数计数并且确保数量不超过被调用函数的参数数量(除非它是一个API函数)

                ++ iParamCount;
                if ( ! pFunc->iIsHostAPI && iParamCount > pFunc->iParamCount )
                    ExitOnCodeError ( "Too many parameters" );

                // Unless this is the final parameter, attempt to read a comma
                // 除非遇到最后一个参数，否则尝试读取一个逗号

                if ( GetLookAheadChar () != ')' )
                    ReadToken ( TOKEN_TYPE_DELIM_COMMA );
            }
            else
            {
                // There isn't, so break the loop and complete the call
                // 函数没有参数，跳出while循环

                break;
            }
        }

        // Attempt to read the closing parenthesis
        // 读取右小括号

        ReadToken ( TOKEN_TYPE_DELIM_CLOSE_PAREN );

        // Make sure the parameter wasn't passed too few parameters (unless
        // it's a host API function)
        // 确保传递了足够数量的参数（除非它是个API函数）

        if ( ! pFunc->iIsHostAPI && iParamCount < pFunc->iParamCount )
            ExitOnCodeError ( "Too few parameters" );

        // Call the function, but make sure the right call instruction is used
        // 调用函数，但是要确保使用了正确的call指令

        int iCallInstr = INSTR_CALL;
        if ( pFunc->iIsHostAPI )
            iCallInstr = INSTR_CALLHOST;

        // 生成call指令的操作数，即函数的index
        int iInstrIndex = AddICodeInstr ( g_iCurrScope, iCallInstr );
        AddFuncICodeOp ( g_iCurrScope, iInstrIndex, pFunc->iIndex );
    }
