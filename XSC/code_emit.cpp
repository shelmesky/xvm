/*

    Project.

        XSC - The XtremeScript Compiler Version 0.8

    Abstract.

        Code emission module

    Date Created.

        9.2.2002

    Author.

        Alex Varanese

*/

// ---- Include Files -------------------------------------------------------------------------

    #include "code_emit.h"

// ---- Globals -------------------------------------------------------------------------------

    FILE * g_pOutputFile = NULL;                        // Pointer to the output file

    // ---- Instruction Mnemonics -------------------------------------------------------------

        // These mnemonics are mapped to each I-code instruction, allowing the emitter to
        // easily translate I-code to XVM assembly

        char ppstrMnemonics [][ 12 ] =
        {
            "Mov",
            "Add", "Sub", "Mul", "Div", "Mod", "Exp", "Neg", "Inc", "Dec",
            "And", "Or", "XOr", "Not", "ShL", "ShR",
            "Concat", "GetChar", "SetChar",
            "Jmp", "JE", "JNE", "JG", "JL", "JGE", "JLE",
            "Push", "Pop",
            "Call", "Ret", "CallHost",
            "Pause", "Exit"
        };

// ---- Functions -----------------------------------------------------------------------------

    /******************************************************************************************
    *
    *   EmitHeader ()
    *
    *   Emits the script's header comments.
    *   生成脚本的头部描述
    */

    void EmitHeader ()
    {
        // Get the current time
        // 获取当前时间

        time_t CurrTimeMs;
        struct tm * pCurrTime;
        CurrTimeMs = time ( NULL );
        pCurrTime = localtime ( & CurrTimeMs );      

        // Emit the filename
        // 生成文件名

        fprintf ( g_pOutputFile, "; %s\n\n", g_pstrOutputFilename );

        // Emit the rest of the header

        fprintf ( g_pOutputFile, "; Source File: %s\n", g_pstrSourceFilename );
        fprintf ( g_pOutputFile, "; XSC Version: %d.%d\n", VERSION_MAJOR, VERSION_MINOR );
        fprintf ( g_pOutputFile, ";   Timestamp: %s\n", asctime ( pCurrTime ) );
    }

    /******************************************************************************************
    *
    *   EmitDirectives ()
    *
    *   Emits the script's directives.
    *   生成脚本的指令
    */

    void EmitDirectives ()
    {
        // If directives were emitted, this is set to TRUE so we remember to insert extra line
        // breaks after them
        // 如果指令已经生成，这个变量会被设置为TRUE，这样我们就可以记得在它们的后面添加换行符

        int iAddNewline = FALSE;

        // If the stack size has been set, emit a SetStackSize directive
        // 如果设置了栈大小，生成一个SetStackSize指令

        if ( g_ScriptHeader.iStackSize )
        {
            fprintf ( g_pOutputFile, "\tSetStackSize %d\n", g_ScriptHeader.iStackSize );
            iAddNewline = TRUE;
        }

        // If the priority has been set, emit a SetPriority directive
        // 如果设置了优先级，生成一个SetProprity指令

        if ( g_ScriptHeader.iPriorityType != PRIORITY_NONE )
        {
            fprintf ( g_pOutputFile, "\tSetPriority " );
            switch ( g_ScriptHeader.iPriorityType )
            {
                // Low rank

                case PRIORITY_LOW:
                    fprintf ( g_pOutputFile, PRIORITY_LOW_KEYWORD );
                    break;

                // Medium rank

                case PRIORITY_MED:
                    fprintf ( g_pOutputFile, PRIORITY_MED_KEYWORD );
                    break;

                // High rank

                case PRIORITY_HIGH:
                    fprintf ( g_pOutputFile, PRIORITY_HIGH_KEYWORD );
                    break;

                // User-defined timeslice

                case PRIORITY_USER:
                    fprintf ( g_pOutputFile, "%d", g_ScriptHeader.iUserPriority );
                    break;
            }
            fprintf ( g_pOutputFile, "\n" );

            iAddNewline = TRUE;
        }

        // If necessary, insert an extra line break
        // 如果有必要，插入一个换行符

        if ( iAddNewline )
            fprintf ( g_pOutputFile, "\n" );
    }

    /******************************************************************************************
    *
    *   EmitScopeSymbols ()
    *
    *   Emits the symbol declarations of the specified scope
    *   为特定的作用域生成符号声明
    */

    void EmitScopeSymbols ( int iScope, int iType )
    {
        // If declarations were emitted, this is set to TRUE so we remember to insert extra
        // line breaks after them
        // 如果生成了声明，就将这个标记设置为TRUE，这样我们就会记得在后面加入换行符
        
        int iAddNewline = FALSE;

        // Local symbol node pointer
        // 局部符号节点指针

        SymbolNode * pCurrSymbol;

        // Loop through each symbol in the table to find the match
        // 从符号表中查找匹配的符号

        for ( int iCurrSymbolIndex = 0; iCurrSymbolIndex < g_SymbolTable.iNodeCount; ++ iCurrSymbolIndex )
        {
            // Get the current symbol structure
            // 获得当前符号结构

            pCurrSymbol = GetSymbolByIndex ( iCurrSymbolIndex );

            // If the scopes and parameter flags match, emit the declaration
            // 如果作用域和参数标记匹配就生成声明

            if ( pCurrSymbol->iScope == iScope && pCurrSymbol->iType == iType )
            {
                // Print one tab stop for global declarations, and two for locals
                // 为全局声明打印一个tab，为局部变量打印两个

                fprintf ( g_pOutputFile, "\t" );
                if ( iScope != SCOPE_GLOBAL )
                    fprintf ( g_pOutputFile, "\t" );

                // Is the symbol a parameter?
                // 这个符号是一个参数？

                if ( pCurrSymbol->iType == SYMBOL_TYPE_PARAM )
                    fprintf ( g_pOutputFile, "Param %s", pCurrSymbol->pstrIdent );

                // Is the symbol a variable?
                // 这个符号是变量？

                if ( pCurrSymbol->iType == SYMBOL_TYPE_VAR )
                {
                    fprintf ( g_pOutputFile, "Var %s", pCurrSymbol->pstrIdent );

                    // If the variable is an array, add the size declaration
                    // 如果是数组就打印大小信息

                    if ( pCurrSymbol->iSize > 1 )
                        fprintf ( g_pOutputFile, " [ %d ]", pCurrSymbol->iSize );
                }

                fprintf ( g_pOutputFile, "\n" );
                iAddNewline = TRUE;
            }
        }

        // If necessary, insert an extra line break
        // 如果有必要加入换行符

        if ( iAddNewline )
            fprintf ( g_pOutputFile, "\n" );
    }

    /******************************************************************************************
    *
    *   EmitFunc ()
    *   
    *   Emits a function, its local declarations, and its code.
    *   生成一个函数，函数的局部声明，和函数的代码。
    */

    void EmitFunc ( FuncNode * pFunc )
    {
        // Emit the function declaration name and opening brace
        // 生成函数的声明和左大括号

        fprintf ( g_pOutputFile, "\tFunc %s\n", pFunc->pstrName );
        fprintf ( g_pOutputFile, "\t{\n" );

        // Emit parameter declarations
        // 生成参数的声明

        EmitScopeSymbols ( pFunc->iIndex, SYMBOL_TYPE_PARAM );

        // Emit local variable declarations
        // 生成局部变量声明

        EmitScopeSymbols ( pFunc->iIndex, SYMBOL_TYPE_VAR );

        // Does the function have an I-code block?
        // 这个函数有中间代码块吗？

        if ( pFunc->ICodeStream.iNodeCount > 0 )
        {
            // Used to determine if the current line is the first
            // 用来确定当前行是否是第一行

            int iIsFirstSourceLine = TRUE;

            // Yes, so loop through each I-code node to emit the code
            // 接下来循环每个中间代码节点生成代码

            for ( int iCurrInstrIndex = 0; iCurrInstrIndex < pFunc->ICodeStream.iNodeCount; ++ iCurrInstrIndex )
            {
                // Get the I-code instruction structure at the current node
                // 获得当前节点的中间代码指令

                ICodeNode * pCurrNode = GetICodeNodeByImpIndex ( pFunc->iIndex, iCurrInstrIndex );

                // Determine the node type
                // 确定节点类型

                switch ( pCurrNode->iType)
                {
                    // Source code annotation
                    // 源代码注释

                    case ICODE_NODE_SOURCE_LINE:
                    {
                        // Make a local copy of the source line
                        // 对源代码进行局部复制

                        char * pstrSourceLine = pCurrNode->pstrSourceLine;

                        // If the last character of the line is a line break, clip it
                        // 如果最后一个字符是换行符，就去掉这个字符

                        int iLastCharIndex = strlen ( pstrSourceLine ) - 1;
                        if ( pstrSourceLine [ iLastCharIndex ] == '\n' )
                            pstrSourceLine [ iLastCharIndex ] = '\0';

                        // Emit the comment, but only prepend it with a line break if it's not the
                        // first one
                        // 生成注释，如果不是第一行的话就预先加入一个换行符

                        if ( ! iIsFirstSourceLine )
                            fprintf ( g_pOutputFile, "\n" );

                        fprintf ( g_pOutputFile, "\t\t; %s\n\n", pstrSourceLine );
                        
                        break;
                    }

                    // An I-code instruction

                    case ICODE_NODE_INSTR:
                    {
                        // Emit the opcode
                        // 生成操作码

                        fprintf ( g_pOutputFile, "\t\t%s", ppstrMnemonics [ pCurrNode->Instr.iOpcode ] );

                        // Determine the number of operands
                        // 确定操作数的个数

                        int iOpCount = pCurrNode->Instr.OpList.iNodeCount;

                        // If there are operands to emit, follow the instruction with some space
                        // 如果有操作数需要生成，那么就在指令的后面加上一些空白

                        if ( iOpCount )
                        {
                            // All instructions get at least one tab
                            // 每个指令最少有一个tab

                            fprintf ( g_pOutputFile, "\t" );

                            // If it's less than a tab stop's width in characters, however, they get a
                            // second
                            // 如果字符串太长，就要再加上一个tab

                            if ( strlen ( ppstrMnemonics [ pCurrNode->Instr.iOpcode ] ) < TAB_STOP_WIDTH )
                                fprintf ( g_pOutputFile, "\t" );
                        }

                        // Emit each operand
                        // 生成每个操作数

                        for ( int iCurrOpIndex = 0; iCurrOpIndex < iOpCount; ++ iCurrOpIndex )
                        {
                            // Get a pointer to the operand structure
                            // 获得操作数结构的指针

                            Op * pOp = GetICodeOpByIndex ( pCurrNode, iCurrOpIndex );

                            // Emit the operand based on its type
                            // 根据类型生成操作数

                            switch ( pOp->iType )
                            {
                                // Integer literal
                                // 整型值

                                case OP_TYPE_INT:
                                    fprintf ( g_pOutputFile, "%d", pOp->iIntLiteral );
                                    break;

                                // Float literal
                                // 浮点值

                                case OP_TYPE_FLOAT:
                                    fprintf ( g_pOutputFile, "%f", pOp->fFloatLiteral );
                                    break;

                                // String literal
                                // 字符串

                                case OP_TYPE_STRING_INDEX:
                                    fprintf ( g_pOutputFile, "\"%s\"", GetStringByIndex ( & g_StringTable, pOp->iStringIndex ) );
                                    break;

                                // Variable
                                // 变量

                                case OP_TYPE_VAR:
                                    fprintf ( g_pOutputFile, "%s", GetSymbolByIndex ( pOp->iSymbolIndex )->pstrIdent );
                                    break;

                                // Array index absolute
                                // 使用绝对索引的数组

                                case OP_TYPE_ARRAY_INDEX_ABS:
                                    fprintf ( g_pOutputFile, "%s [ %d ]", GetSymbolByIndex ( pOp->iSymbolIndex )->pstrIdent,
                                                                          pOp->iOffset );
                                    break;

                                // Array index variable
                                // 变量索引的数组

                                case OP_TYPE_ARRAY_INDEX_VAR:
                                    fprintf ( g_pOutputFile, "%s [ %s ]", GetSymbolByIndex ( pOp->iSymbolIndex )->pstrIdent,
                                                                          GetSymbolByIndex ( pOp->iOffsetSymbolIndex )->pstrIdent );
                                    break;

                                // Function
                                // 函数

                                case OP_TYPE_FUNC_INDEX:
                                    fprintf ( g_pOutputFile, "%s", GetFuncByIndex ( pOp->iSymbolIndex )->pstrName );
                                    break;

                                // Register (just _RetVal for now)
                                // 寄存器（现在只有 _RetVal)

                                case OP_TYPE_REG:
                                    fprintf ( g_pOutputFile, "_RetVal" );
                                    break;

                                // Jump target index
                                // 跳转目标索引

                                case OP_TYPE_JUMP_TARGET_INDEX:
                                    fprintf ( g_pOutputFile, "_L%d", pOp->iJumpTargetIndex );
                                    break;
                            }

                            // If the operand isn't the last one, append it with a comma and space
                            // 如果这个操作数不是最后一个，就在后面加上一个逗号和空格

                            if ( iCurrOpIndex != iOpCount - 1 )
                                fprintf ( g_pOutputFile, ", " );
                        }

                        // Finish the line
                        // 完成该行

                        fprintf ( g_pOutputFile, "\n" );

                        break;
                    }

                    // A jump target
                    // 跳转目标

                    case ICODE_NODE_JUMP_TARGET:
                    {
                        // Emit a label in the format _LX, where X is the jump target
                        // 以_LX格式生成标号，这里的X就是跳转目标

                        fprintf ( g_pOutputFile, "\t_L%d:\n", pCurrNode->iJumpTargetIndex );
                    }
                }

                // Update the first line flag
                // 更新第一个行标记

                if ( iIsFirstSourceLine )
                    iIsFirstSourceLine = FALSE;
            }
        }
        else
        {
            // No, so emit a comment saying so
            // 函数不含代码，所以生成一个注释说明

            fprintf ( g_pOutputFile, "\t\t; (No code)\n" );
        }

        // Emit the closing brace
        // 生成右括号

        fprintf ( g_pOutputFile, "\t}" );
    }

    /******************************************************************************************
    *
    *   EmitCode ()
    *
    *   Translates the I-code representation of the script to an ASCII-foramtted XVM assembly
    *   file.
    *
    *   将脚本的中间代码表示转换为ASCII格式的XVM汇编文件。
    */

    void EmitCode ()
    {
        // ---- Open the output file
        // ---- 打开输出文件

        if ( ! ( g_pOutputFile = fopen ( g_pstrOutputFilename, "wb" ) ) )
            ExitOnError ( "Could not open output file for output" );

        // ---- Emit the header
        // ---- 生成文件头

        EmitHeader ();

        // ---- Emit directives
        // ---- 生成命令

        fprintf ( g_pOutputFile, "; ---- Directives -----------------------------------------------------------------------------\n\n" );

        EmitDirectives ();

        // ---- Emit global variable declarations
        // ---- 生成全局变量声明

        fprintf ( g_pOutputFile, "; ---- Global Variables -----------------------------------------------------------------------\n\n" );

        // Emit the globals by printing all non-parameter symbols in the global scope
        // 通过打印所有全局范围的非参数符号来生成全局变量

        EmitScopeSymbols ( SCOPE_GLOBAL, SYMBOL_TYPE_VAR );

        // ---- Emit functions
        // ---- 生成函数

        fprintf ( g_pOutputFile, "; ---- Functions ------------------------------------------------------------------------------\n\n" );

        // Local node for traversing lists
        // 遍历链表的局部节点

        LinkedListNode * pNode = g_FuncTable.pHead;

        // Local function node pointer
        // 局部函数节点指针

        FuncNode * pCurrFunc;

        // Pointer to hold the _Main () function, if it's found
        // 如果发现了_Main()函数，就用这个指针保存

        FuncNode * pMainFunc = NULL;

        // Loop through each function and emit its declaration and code, if functions exist
        // 生成每个函数的声明和代码

        if ( g_FuncTable.iNodeCount > 0 )
        {
            while ( TRUE )
            {
                // Get a pointer to the node
                // 获得节点指针

                pCurrFunc = ( FuncNode * ) pNode->pData;

                // Don't emit host API function nodes
                // 不能生成主应用程序API函数节点

                if ( ! pCurrFunc->iIsHostAPI )
                {
                    // Is the current function _Main ()?
                    // 当前函数是_Main()

                    if ( strcasecmp ( pCurrFunc->pstrName, MAIN_FUNC_NAME ) == 0 )
                    {
                        // Yes, so save the pointer for later (and don't emit it yet)
                        // 是的，所以保存这个指针以备后用

                        pMainFunc = pCurrFunc;
                    }
                    else
                    {
                        // No, so emit it
                        // 不是，那么生成

                        EmitFunc ( pCurrFunc );
                        fprintf ( g_pOutputFile, "\n\n" );
                    }
                }

                // Move to the next node
                // 处理下一个节点

                pNode = pNode->pNext;
                if ( ! pNode )
                    break;
            }
        }

        // ---- Emit _Main ()
        // ---- 生成 _Main()
    
        fprintf ( g_pOutputFile, "; ---- Main -----------------------------------------------------------------------------------" );

        // If the last pass over the functions found a _Main () function. emit it
        // 如果发现了函数_Main()，那就生成这个函数

        if ( pMainFunc )
        {
            fprintf ( g_pOutputFile, "\n\n" );
            EmitFunc ( pMainFunc );
        }

        // ---- Close output file
        // ---- 关闭输出文件

        fclose ( g_pOutputFile );
    }
