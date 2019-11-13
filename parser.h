#pragma once

#include "common.h"
#include "error_handling.h"
#include "lexer.h"
#include "ast.h"

inline ret_type
ParseTypedEnum()
{
    token      = GetToken(&lexer);
    peek_token = PeekToken(&lexer);
    
    if (peek_token == Token_Identifier)
    {
        token      = GetToken(&lexer);
        peek_token = PeekToken(&lexer);
        
        if (peek_token.type == Token_OpenBrace)
        {
            token      = GetToken(&lexer);
            peek_token = PeekToken(&lexer);
            
            // ParseEnumBody
        }
        
        else
        {
            //// ERROR: Unexpected token in enum declaration
        }
    }
    
    else
    {
        //// ERROR: Expected type name after colon in enum declaration
    }
}

inline ret_type
ParseStringStream(String_Stream stream)
{
    Lexer lexer = LexStringStream(stream);
    
    do
    {
        /*
** NOTE(soimn): This is a mock grammar and may not be remotely correct
**
** translation_unit : external_decl_or_def translation_unit
**                  | external_decl_or_def
**                  ;
** 
** external_decl_or_def : type_decl
**                      | type_def
**                      | function_decl
**                      | function_def
**                      | var_decl
**                      ;
**
** type_decl : STRUCT ident ';'
**           | UNION ident ';'
**           ;
**
** type_def : STRUCT ident '{' struct_or_union_body '}' ';'
**          | STRUCT '{' struct_or_union_body '}' ';'
**          | UNION ident '{' struct_or_union_body '}' ';'
**          | UNION '{' struct_or_union_body '}' ';'
**          | ENUM ident '{' enum_body '}' ';'
**          | ENUM '{' enum_body '}' ';'
**          | typedef type ident ';'
**          | typedef type_def ident ';'
**          ;
**
** function_decl : tags linkage_tags type ident '(' function_param_list ')' ';';
** function_def  : tags linkage_tags type ident '(' function_param_list ')' ';' function_body;
**
** var_decl : tags type ident, additional_var_decls ';'
**          | tags type ident '=' expression, additional_var_decl ';'
**          | tags type ident ';'
**          | tags type ident '=' expression ';'
**          ;
*/
        
        Token token  = GetToken(&lexer);
        Token peek_token = PeekToken(&lexer);
        
        if (token.type == Token_Struct || token.type == Token_Union)
        {
            if (peek_token.type == Token_Identifier)
            {
                /// Named struct or union
            }
            
            else if (peek_token.type == Token_OpenBrace)
            {
                /// Unnamed struct or union
            }
            
            else
            {
                //// ERROR: Unexpected token after the struct/union keyword
            }
        }
        
        else if (token.type == Token_Enum)
        {
            if (peek_token.type == Token_Identifier)
            {
                /// Named enum declaration
                
                token      = GetToken(&lexer);
                peek_token = PeekToken(&lexer);
                
                if (peek_token.type == Token_Colon)
                {
                    /// Named and typed enum declaration
                    
                    // ParseTypedEnum();
                }
                
                else if (peek_token.type == Token_OpenBrace)
                {
                    /// Named but not typed enum declaration
                    
                    token      = GetToken(&lexer);
                    peek_token = PeekToken(&lexer);
                    
                    // ParseEnumBody();
                }
                
                else
                {
                    //// ERROR: Unexpected token in enum declaration
                }
            }
            
            else if (peek_token.type == Token_Colon)
            {
                /// Unnamed typed enum declaration
                
                // ParseTypedEnum();
            }
            
            else if (peek_token.type == Token_OpenBrace)
            {
                /// Unnamed but not typed enum declaration
                
                // ParseEnumBody();
            }
            
            else
            {
                //// ERROR: Unexpected token after the enum keyword
            }
        }
        
        else if (token.type == Token_Typedef)
        {
            
        }
        
        else
        {
            //// ERROR: encountered an unknown token in global scope
        }
        
    } while (token.type != Token_EndOfStream);
}