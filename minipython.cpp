//Shaz Maknojia and Jeff John


#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <map>
#include <algorithm>
#include <tuple>
#include <regex>
#include <algorithm>
#include <iostream>
#include <string>
#include <math.h>


std::vector<std::string> readFile(std::string fileName){
    std::ifstream file(fileName);
    std::vector<std::string> content;
    std::string line;
    while (std::getline (file, line)) {
        content.push_back(line);
    }

    return content;
}


class token{
    public:
        std::string type;
        std::string value;
        token *nest = NULL;
        int line;
        int indent;
        token(std::string type, std::string value){
            this->type = type;
            this->value = value;
            this->nest = NULL;
        }
        token(){
            this->type = "";
            this->value = "";
            this->nest = NULL;
        }
        token(token *t){
            this->type = t->type;
            this->value = t->value;
            this->nest = t->nest;
        }
};

// class funct that is a vector of tokens
class funct{
    public:
        token* arguments;
        std::vector<token> body;

        std::string name;
        funct(std::string name, std::vector<token> tokens,token* variables){
            this->name = name;
            this->body = tokens;
            this->arguments = variables;
        }
        funct(){
            this->name = "";
            this->body = {};
            this->arguments = NULL;
        }
};

token parser_helper(token &tokens, std::map<std::string, token> &variables, std::vector<funct> &all_functions);
token recursive_parser(std::vector<funct> &all_functions, std::string &funct_name, std:: map<std::string, token> &variables);


// helper for Lexer
token lexHelper(std::string content, int line_num){
    std::smatch match;
    std::string line;
    token t;
    
    // regular expressions to recognize identifiers (var/function names), numbers and strings
    std::regex identifier("[a-zA-Z_][a-zA-Z0-9_]*");
    std::regex number("[0-9]+");
    std::regex arithmeticExpression("^([a-zA-Z()_\\[\\] ][a-zA-Z()0-9 _\\[\\]]*|[0-9][0-9]*)[ ]*[\\+\\*\\-\\/][ ]*([a-zA-Z()_ \\[\\]][a-zA-Z0-9()_ \\[\\]]*|[0-9][0-9]*)([ ]*[\\+\\*\\-\\/][ ]*([a-zA-Z()\\[\\] _][a-zA-Z0-9() _\\[\\]]*|[0-9][0-9]*))*$");
    std::regex variableDeclaration("^[ ]*([a-zA-Z_][a-zA-Z0-9_]*)[ ]*=[ ]*(.*?)[ ]*$");
    std::regex printStatement("print[ ]*[ ]*\\((.*?)\\)[ ]*$");
    std::regex pythonList("\\[(.*?)\\]");
    std::regex ifStatement("if[ ]*[\\(]?([a-zA-Z_][a-zA-Z0-9_\\[\\]]*|[0-9]+)[ ]*([<|>|=|!]{2}|[<|>|!])[ ]*([a-zA-Z_][a-zA-Z0-9_\\[\\]]*|[0-9]+)[ ]*[\\)]?[ ]*:");
    std::regex arrayAddition("^\\[[ ]*([0-9]+[ ]*,[ ]*)*[0-9]+[ ]*\\][ ]*[+][ ]*\\[[ ]*([0-9]+[ ]*,[ ]*)*[0-9]+[ ]*\\]([ ]*[+][ ]*\\[[ ]*([0-9]+[ ]*,[ ]*)*[0-9]+[ ]*\\])*[ ]*$");
    // regrx for list index assignment ex: list[0] = 1
    std::regex arrayIndexAssignment("^([a-zA-Z_][a-zA-Z0-9_]*)[ ]*\\[[ ]*([0-9 \\:]+)[ ]*\\][ ]*=[ ]*(.*?)[ ]*$");
    // regex for touple without parenthesis ex: "aa", 1, 2, "h"
    std::regex ungroupedTuple("^([a-zA-Z_=\"\'][a-zA-Z0-9_=\"\' :]*|[0-9]+)[ ]*[,][ ]*(.*?)$");
    // regex for string
    std::regex string("\"[a-zA-Z0-9_:= ]*\"");
    // regex for else:
    std::regex elseStatement("else[ ]*:");
    // regex for array index access
    std::regex arrayIndexAccess("^([a-zA-Z_][a-zA-Z0-9_]*)[ ]*\\[[ ]*([a-zA-Z_:][a-zA-Z0-9_:]*|[0-9 :]+)[ ]*\\][ ]*$");
    // ^def[ ]*([a-zA-Z_]*)\(([a-zA-Z_, 1-9]*)\):
    std::regex functionDeclaration("^def[ ]*([a-zA-Z_]*)[ ]*\\(([a-zA-Z_, 0-9]*)\\):$");
    // ^[a-zA-Z_][a-zA-Z_1-9]*\(([a-zA-Z_1-9, ]*)\)$
    std::regex functionCall("^([a-zA-Z()_][a-zA-Z_0-9()]*)[ ]*\\(([a-zA-Z()_, 0-9]*)\\)$");
    // return(.*?)$
    // regex for return statement with or without parenthesis
    std::regex returnStatement("return[ ]*(.*?)[ ]*$");
    std::regex index("([:]+)");

    line = content;
    int indent = 0;
    while(line[indent] == ' '){
        indent++;
    }
    t.indent = indent;

    // also check for # in the middle of the line
    if (line.find("#") != std::string::npos){
        t.type = "comment";
        t.value = line;
    }

    // check if the line is a function declaration
    else if (std::regex_match(line, match, functionDeclaration)){
        t.type = "functionDeclaration";
        t.value = match[1];
        t.nest = new token(lexHelper(match[2].str(), line_num));
    }
    // check if the line is a ifStatement
    else if (std::regex_search(line, match, ifStatement)){
        t.type = "ifStatement";
        t.value = match[2].str();

        token t2(lexHelper(match[1].str(), line_num));
        token t3(lexHelper(match[3].str(), line_num));
        
        t.nest = new token(t2);
        t.nest->nest = new token(t3);
    }
    // check if the line is a elseStatement
    else if (std::regex_search(line, match, elseStatement)){
        t.type = "elseStatement";
        t.value = match[0].str();
    }
    // check if the line is a return statement
    else if (std::regex_search(line, match, returnStatement)){
        t.type = "returnStatement";
        t.value = "return";
        token t2(lexHelper(match[1].str(), line_num));
        t.nest = new token(t2);
    }
    // check if the line is a print statement
    else if (std::regex_search(line, match, printStatement)){
        std::string inside_pars = match[1].str();   
        t.nest = new token(lexHelper(inside_pars, line_num));
        t.type = "printStatement";
        t.indent = indent;
    }
    // check if the line is a function call
    else if (std::regex_match(line, match, functionCall)){
        t.type = "functionCall";
        t.value = match[1];
        t.nest = new token(lexHelper(match[2].str(), line_num));
    }
    // check if the line is a arrayIndexAssignment
    else if (std::regex_match(line, match, arrayIndexAssignment)){
        t.type = "arrayIndexAssignment";
        t.value = match[1].str() + " " + match[2].str();
        t.nest = new token(lexHelper(match[3].str(), line_num));
    }
    // check if the line is a variable declaration
    else if (std::regex_search(line, match, variableDeclaration)){
        std::string variableName = match[1].str();
        std::string variableValue = match[2].str();

        t = token("variableDeclaration", variableName);
        t.nest = new token(lexHelper(variableValue, line_num));
    }
    // check if the line is an array addition
    else if (std::regex_search(line, match, arrayAddition)){
        std::string expression = match[0].str();
        t = token("arrayAddition", expression);
    }
    // check if the line is a arrayIndexAccess
    else if (std::regex_search(line, match, arrayIndexAccess)){
        std::string arrayName = match[1].str();
        std::string index = match[2].str();
        t = token("arrayIndexAccess", arrayName + " " + index);
    }
    // check if the line is an arithmetic expression
    else if (std::regex_search(line, match, arithmeticExpression)){
        std::string expression = match[0].str();
        t = token("arithmeticExpression", expression);
    }
    // check if the line is a list
    else if (std::regex_search(line, match, pythonList)){
        std::string list = match[1].str();
        t = token("pythonList", list);
    }
    // check if the line is a touple
    else if (std::regex_search(line, match, ungroupedTuple)){
        std::string touple = match[1].str();
        t = token(lexHelper(touple, line_num));
        t.nest = new token(lexHelper(match[2].str(), line_num));
    }
    // check if the line is a string
    else if (std::regex_search(line, match, string)){
        std::string string = line.substr(1, line.length()-2);
        t = token("string", string);
    }
    // check if the line is an identifier
    else if (std::regex_search(line, match, identifier)){
        std::string identifier = match[0].str();
        t = token("identifier", identifier);
    }
    // check if the line is an index
    else if (std::regex_search(line, match, index)){
        t = token("index", line);
    }
    // check if the line is a number
    else if (std::regex_search(line, match, number)){
        std::string number = match[0].str();
        t = token("number", number);
    }
    // edge cases
    else if (line == "") t = token("empty", "");
    else t = token("unknown", line);

    // calculate the indentation level
    t.indent = indent;
    t.line = line_num;

    //print the token
    // std::cout << t.type << " " << t.value << std::endl;

    return t;
}

// lexer inputted python code using regular expressions
std::vector<token> lexer(std::vector<std::string> content){
    std::vector<token> tokens;

    // loop through each line of the file and store token* in a vector of tokens
    for (int i = 0; i < content.size(); i++){
        token t = lexHelper(content[i], i+1);
        tokens.push_back(t);
    }

    // //print the tokens
    // for (int i = 0; i < tokens.size(); i++){
    //     std::cout << tokens[i].type << " " << tokens[i].value << std::endl;
    //     // if it has a nest, print that too
    //     if (tokens[i].nest != NULL){
    //         std::cout << "    " << tokens[i].nest->type << " " << tokens[i].nest->value << std::endl;
    //     }
    // }

    return tokens;
}

std::string expression_solver(std::string s){
    std::string left;
    std::string right;

    ///    12 * 3 + 12 * 3 

    if (s.find("+") != std::string::npos){
        // split the string into two parts
        std::string left = s.substr(0, s.find("+"));
        std::string right = s.substr(s.find("+") + 1, s.length());
        // recursively solve the left and right parts
        left = expression_solver(left);
        right = expression_solver(right);
        // return the sum of the left and right parts
        return std::to_string(std::stoi(left) + std::stoi(right));
    }
    else if (s.find("-") != std::string::npos){
        // split the string into two parts
        std::string left = s.substr(0, s.find("-"));
        std::string right = s.substr(s.find("-") + 1, s.length());
        // recursively solve the left and right parts
        left = expression_solver(left);
        right = expression_solver(right);
        // return the difference of the left and right parts
        return std::to_string(std::stoi(left) - std::stoi(right));
    }
    else if (s.find("*") != std::string::npos){
        // split the string into two parts
        std::string left = s.substr(0, s.find("*"));
        std::string right = s.substr(s.find("*") + 1, s.length());
        // recursively solve the left and right parts
        left = expression_solver(left);
        right = expression_solver(right);
        // return the product of the left and right parts
        return std::to_string(std::stoi(left) * std::stoi(right));
    }
    else if (s.find("/") != std::string::npos){
        // split the string into two parts
        std::string left = s.substr(0, s.find("/"));
        std::string right = s.substr(s.find("/") + 1, s.length());
        // recursively solve the left and right parts
        left = expression_solver(left);
        right = expression_solver(right);
        // return the quotient of the left and right parts
        return std::to_string(std::stoi(left) / std::stoi(right));
    }
    else if (s.find("^") != std::string::npos){
        // split the string into two parts
        std::string left = s.substr(0, s.find("^"));
        std::string right = s.substr(s.find("^") + 1, s.length());
        // recursively solve the left and right parts
        left = expression_solver(left);
        right = expression_solver(right);
        // return the exponent of the left and right parts
        return std::to_string(std::pow(std::stoi(left), std::stoi(right)));
    }
    else{
        return s;
    } 
}

std::string replace_variables_for_pyhon_list(std::string line, std::map<std::string, token> &variables){
    std::string item;
    std::vector<std::string> items;
    // regex to match identifier
    std::regex identifier("[a-zA-Z_][a-zA-Z0-9_]*");
    std::regex arrayIndexAccess("^([a-zA-Z_][a-zA-Z0-9_]*)[ ]*\\[[ ]*([0-9]+)[ ]*\\][ ]*$");


    // remove brackets from the pythonList
    line = std::regex_replace(line, std::regex("^\\[|\\]$"), "");

    // if the line is empty return an empty string after adding the brackets back
    if (line == ""){
        return "[]";
    }

    // if the line is one item return the item with brackets after replacing the variables
    if (std::regex_match(line, identifier)){
        if (variables.find(line) != variables.end()){
            return "[" + variables[line].value + "]";
        }
        else{
            return "[" + line + "]";
        }
    }

    // split the line into items
    for (int j = 0; j < line.length(); j++){
        if (line.substr(j, 1) == ","){
            if (item != ""){
                items.push_back(item);
                item = "";
            }
            items.push_back(line.substr(j, 1));
        }
        else{
            item += line.substr(j, 1);
        }
    }

    // add the last item
    if (item != ""){
        items.push_back(item);
    }

    // replace the variables with their values if a variable is found
    for (int j = 0; j < items.size(); j++){
        if (std::regex_match(items[j], identifier)){
            if (variables.find(items[j]) != variables.end()){
                items[j] = variables[items[j]].value;
            }
        }
    }

    // todo: find any arrayIndexAccess is in the items and replace it with the value of the array at that index

    // join the items back into a string adding the brackets and commas back
    std::string new_line = "[";
    for (int j = 0; j < items.size(); j++){
        new_line += items[j];
    }
    new_line += "]";
    return new_line;

}

// replace the variables with their values
// variables can be separated by spaces or operators
// input is a string arithemtic expression ex: "a + b * cat + 12"
std::string replace_variables(std::string line, std::map<std::string, token> &variables, int &i){
    std::string item;
    std::vector<std::string> items;
    // regex to match identifier
    std::regex identifier("[a-zA-Z_][a-zA-Z0-9_]*");
    std::regex arrayIndexAccess("^([a-zA-Z_][a-zA-Z0-9_]*)[ ]*\\[[ ]*([0-9]+)[ ]*\\][ ]*$");
    std::regex pythonList("\\[(.*?)\\]");

    // make map of operators
    std::map<std::string, bool> operators;
    operators["+"] = true;
    operators["-"] = true;
    operators["*"] = true;
    operators["/"] = true;
    operators["^"] = true;
    operators[" "] = true;

    // split the line into items
    for (int j = 0; j < line.length(); j++){
        if (operators.find(line.substr(j, 1)) != operators.end()){
            if (item != ""){
                items.push_back(item);
                item = "";
            }
            items.push_back(line.substr(j, 1));
        }
        else{
            item += line.substr(j, 1);
        }
    }

    // add the last item
    if (item != ""){
        items.push_back(item);
    }

    // if an item is a pythonList and it contains a letter run the replace_variables_for_pyhon_list function
    for (int j = 0; j < items.size(); j++){
        if (std::regex_match(items[j], pythonList)){
            if (items[j].find_first_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") != std::string::npos){
                items[j] = replace_variables_for_pyhon_list(items[j], variables);
            }
        }
    }


    // replace the variables with their values if a variable is found and it is a "pythonList" set i to 1
    for (int j = 0; j < items.size(); j++){
        if (std::regex_match(items[j], identifier)){
            if (variables.find(items[j]) != variables.end()){
                if (variables[items[j]].type == "pythonList"){
                    i = 1;
                }
                items[j] = variables[items[j]].value;
            }
        }
    }

    // find any arrayIndexAccess is in the items and replace it with the value of the array at that index
    // ex: a[0] -> 1
    for (int j = 0; j < items.size(); j++){
        if (std::regex_match(items[j], arrayIndexAccess)){
            std::smatch match;
            std::regex_search(items[j], match, arrayIndexAccess);
            std::string arrayName = match[1];
            std::string index = match[2];
            if (variables.find(arrayName) != variables.end()){
                std::vector<std::string> items2;
                std::string item;
                std::string pl = variables[arrayName].value;
                // remove the brackets from the pythonList
                pl = std::regex_replace(pl, std::regex("^\\[|\\]$"), "");
                std::stringstream ss(pl);

                while (std::getline(ss, item, ',')){
                    items2.push_back(item);
                }

                // get the item at the index
                std::string i = items2[std::stoi(index)];


                // replace the arrayIndexAccess with the value at the index
                items[j] = i;

            }
        }
    }


    // join the items back into a string preserving the spaces and operators
    std::string new_line;
    for (int j = 0; j < items.size(); j++){
        new_line += items[j];
    }
    return new_line;

}

std::string replace_functions(std::string line, std::vector<funct> all_functions){
    // regex to match identifier
    std::regex identifier("[a-zA-Z_][a-zA-Z0-9_]*");
    std::regex functionCall("^([a-zA-Z_][a-zA-Z0-9_]*)[ ]*\\((.*)\\)[ ]*$");

    // make map of operators
    std::map<std::string, bool> operators;
    operators["+"] = true;
    operators["-"] = true;
    operators["*"] = true;
    operators["/"] = true;
    operators["^"] = true;
    operators[" "] = true;

    // split the line into items
    std::string item;
    std::vector<std::string> items;
    for (int j = 0; j < line.length(); j++){
        if (operators.find(line.substr(j, 1)) != operators.end()){
            if (item != ""){
                items.push_back(item);
                item = "";
            }
            items.push_back(line.substr(j, 1));
        }
        else{
            item += line.substr(j, 1);
        }
    }

    // add the last item
    if (item != ""){
        items.push_back(item);
    }

    // replace the functions with their values if a function is found
    for (int j = 0; j < items.size(); j++){
        if (std::regex_match(items[j], functionCall)){
            std::smatch match;
            std::regex_search(items[j], match, functionCall);
            std::string functionName = match[1];
            std::string functionArgs = match[2];
            for (int k = 0; k < all_functions.size(); k++){
                if (all_functions[k].name == functionName){
                    // create a map of the function arguments
                    std::map<std::string, token> function_variables;
                    std::vector<std::string> functionArgsVector;
                    std::string item;
                    std::stringstream ss(functionArgs);
                    // if there are multiple arguments split them into a vector
                    if (functionArgsVector.size() == 0){
                        functionArgsVector.push_back(functionArgs);
                    }
                    else{
                        while (std::getline(ss, item, ',')){
                            functionArgsVector.push_back(item);
                        }
                    }
  
                    // add the function arguments to the map using the names from the funct linked list
                    token t = all_functions[k].arguments;

                    for (int l = 0; l < functionArgsVector.size(); l++){
                        function_variables[t.value] = token(t.type, functionArgsVector[l]);
                        if (t.nest != NULL){
                            t = *t.nest;
                        }
                    }

                    // run the function using recursive_parser
                    token result = recursive_parser(all_functions, all_functions[k].name, function_variables);
                    // replace the function call with the result
                    items[j] = result.value;

                }
            }
        }
    }

    // join the items back into a string preserving the spaces and operators
    std::string new_line;
    for (int j = 0; j < items.size(); j++){
        new_line += items[j];
    }
    return new_line;

}



// concatonate n python lists together
// input is a string of python lists separated by "+" operators
std::string array_addition(std::string line, int line_number){
    // split the line into items
    std::vector<std::string> items;
    std::string item;
    std::stringstream ss(line);

    // if number is attampted to be added to a pythonList return an error and print the line number
    if (std::regex_search(line, std::regex("\\][ ]*[+-]{1}[ ]*[0-9]+"))){
        std:: cout << "Error: Cannot add a number to a pythonList on line " << line_number << std::endl;
        exit(1);
    }


    while (std::getline(ss, item, '+')){
        items.push_back(item);
    }
    // remove any whitespace before and after the items
    for (int i = 0; i < items.size(); i++){
        items[i] = std::regex_replace(items[i], std::regex("^\\s+|\\s+$"), "");
    }


    // remove any brackets from the items
    for (int i = 0; i < items.size(); i++){
        items[i] = std::regex_replace(items[i], std::regex("^\\[|\\]$"), "");
    }


    // concatonate the items together and add ", " between them except for the last item
    std::string new_line = "";
    for (int i = 0; i < items.size(); i++){
        new_line += items[i];
        if (i != items.size() - 1){
            new_line += ", ";
        }
    }

    // add brackets to the front and back of the result
    new_line = "[" + new_line + "]";

    // replace "[, " with "[" and ", ]" with "]"
    new_line = std::regex_replace(new_line, std::regex("\\[, "), "[");
    new_line = std::regex_replace(new_line, std::regex(", \\]"), "]");


    return new_line;
}

// recursive parser helper function
token parser_helper(token &tokens, std::map<std::string, token> &variables, std::vector<funct> &all_functions){
    // if the token is an if statement
    if (tokens.type == "ifStatement"){
        // get operator from tokens.value
        std::string op = tokens.value;
        // get the left and right sides of the operator
        token left_ = *tokens.nest;
        token right_ = *tokens.nest->nest;

        // recursively parse the left and right sides using parser_helper
        left_ = parser_helper(left_, variables, all_functions);
        right_ = parser_helper(right_, variables, all_functions);

        std :: string left = left_.value;
        std :: string right = right_.value;

        // print left and right for debugging
        // todo: add support for solving expressions

        // if else statements to handle the different operators
        if (op == "=="){
            if (left == right) return token("ifStatementResult", "true");
            else return token("ifStatementResult", "false");
        }
        else if (op == "!="){
            if (left != right) return token("ifStatementResult", "true");
            else return token("ifStatementResult", "false");
        }
        else if (op == ">"){
            if (std::stoi(left) > std::stoi(right)) return token("ifStatementResult", "true");
            else return token("ifStatementResult", "false");
        }
        else if (op == "<"){
            if (std::stoi(left) < std::stoi(right)) return token("ifStatementResult", "true");
            else return token("ifStatementResult", "false");
        }
        else if (op == ">="){
            if (std::stoi(left) >= std::stoi(right)) return token("ifStatementResult", "true");
            else return token("ifStatementResult", "false");
        }
        else if (op == "<="){
            if (std::stoi(left) <= std::stoi(right)) return token("ifStatementResult", "true");
            else return token("ifStatementResult", "false");
        }
        else{
            std::cerr << "Error: invalid operator" << std::endl;
            return token("error", "invalid operator");
        }
    }
    // if the token is an else statement
    if (tokens.type == "elseStatement"){
        return token("elseStatement", "N/A");
    }
    // if the token is a return statement
    if (tokens.type == "returnStatement"){
        // get the value of the return statement
        token result = new token("returnStatementResult", "None");
        result.nest = new token(parser_helper(*tokens.nest, variables, all_functions));
        return result;
    }
    // if the token is an arrayIndexAssignment
    if (tokens.type == "arrayIndexAssignment"){
        // tokens.value is the array name and the index separated by a space ex: "a 0"
        // tokens.next is the value to assign to the index

        // split the tokens.value into the array name and the index
        std::string array_name = tokens.value.substr(0, tokens.value.find(" "));
        std::string index = tokens.value.substr(tokens.value.find(" ") + 1, tokens.value.length());

        // if the tokens.nest is an identifier then replace it with its value
        tokens.nest = new token(parser_helper(*tokens.nest, variables, all_functions));


        // if the array name is a variable and it is a pythonList replace the lists value at the index with the value of tokens.nest
        if (variables.find(array_name) != variables.end()){
            
            if (variables[array_name].type == "pythonList"){
                // split the pythonList into items
                std::vector<std::string> items;
                std::string item;
                std::string pl = variables[array_name].value;
                // remove the brackets from the pythonList
                pl = std::regex_replace(pl, std::regex("^\\[|\\]$"), "");
                std::stringstream ss(pl);


                while (std::getline(ss, item, ',')){
                    items.push_back(item);
                }

                // remove any whitespace before and after the items
                for (int i = 0; i < items.size(); i++){
                    items[i] = std::regex_replace(items[i], std::regex("^\\s+|\\s+$"), "");
                }

                // if the index conttains a ":" then it is a slice
                if (index.find(":") != std::string::npos){
                    // split the index into the start and end of the slice
                    std::string start = index.substr(0, index.find(":"));
                    std::string end = index.substr(index.find(":") + 1, index.length());

                    // if the start is empty then set it to 0
                    if (start == "") start = "0";

                    // if the end is empty then set it to the length of the list
                    if (end == "") end = std::to_string(items.size());

                    // if it is only a start index then replace the items after the start index with the value of tokens.nest
                    // the value of tokens.nest should be a pythonList
                    // remove the brackets from the pythonList before splitting it into items
                    if (end == std::to_string(items.size())){
                        std::string pl = tokens.nest->value;
                        pl = std::regex_replace(pl, std::regex("^\\[|\\]$"), "");
                        std::stringstream ss(pl);
                        std::vector<std::string> new_items;
                        while (std::getline(ss, item, ',')){
                            new_items.push_back(item);
                        }

                        // remove any whitespace before and after the items
                        for (int i = 0; i < new_items.size(); i++){
                            new_items[i] = std::regex_replace(new_items[i], std::regex("^\\s+|\\s+$"), "");
                        }

                        // replace the items after the start index with the items in the pythonList
                        for (int i = std::stoi(start); i < items.size(); i++){
                            items[i] = new_items[i - std::stoi(start)];
                        }
                    }

                    // if it is only an end index then replace the items before the end index with the value of tokens.nest
                    // the value of tokens.nest should be a pythonList
                    // remove the brackets from the pythonList before splitting it into items
                    else if (start == "0"){
                        std::string pl = tokens.nest->value;
                        pl = std::regex_replace(pl, std::regex("^\\[|\\]$"), "");
                        std::stringstream ss(pl);
                        std::vector<std::string> new_items;
                        while (std::getline(ss, item, ',')){
                            new_items.push_back(item);
                        }

                        // remove any whitespace before and after the items
                        for (int i = 0; i < new_items.size(); i++){
                            new_items[i] = std::regex_replace(new_items[i], std::regex("^\\s+|\\s+$"), "");
                        }

                        // replace the items before the end index with the items in the pythonList
                        for (int i = 0; i < std::stoi(end); i++){
                            items[i] = new_items[i];
                        }
                    }

                    // todo: other slice types

                    // parse the items back into a pythonList
                    std::string new_pl = "[";
                    for (int i = 0; i < items.size(); i++){
                        new_pl += items[i];
                        if (i != items.size() - 1) new_pl += ", ";
                    }
                    new_pl += "]";
                    variables[array_name].value = new_pl;


                }
                else{
                    // replace the item at the index with the value of tokens.next
                    items[std::stoi(index)] = tokens.nest->value;

                    // join the items back into a string and add ", " between them except for the last item
                    std::string new_line = "";
                    for (int i = 0; i < items.size(); i++){
                        new_line += items[i];
                        if (i != items.size() - 1){
                            new_line += ", ";
                        }
                    }

                    // add brackets to the front and back of the result
                    new_line = "[" + new_line + "]";

                    // set the value of the pythonList to the new value
                    variables[array_name].value = new_line;
                }
                
            }
        }
    }
    // if the token is a arrayIndexAccess
    if (tokens.type == "arrayIndexAccess"){
        // split the tokens.value into the array name and the index
        std::string array_name = tokens.value.substr(0, tokens.value.find(" "));
        std::string index = tokens.value.substr(tokens.value.find(" ") + 1, tokens.value.length());


        // if the index conttains a letter then it is a variable replace it with its value
        if (std::regex_match(index, std::regex("[a-zA-Z]+"))){
            index = variables[index].value;
        }
        
        // if index contains a ":" then it is a slice
        if (index.find(':') != std::string::npos){
            // split the index into the start and end
            std::string start = index.substr(0, index.find(":"));
            std::string end = index.substr(index.find(":") + 1, index.length());

            // if the array name is a variable and it is a pythonList replace the lists value at the index with the value of tokens.nest
            if (variables.find(array_name) != variables.end()){
                
                if (variables[array_name].type == "pythonList"){
                    // split the pythonList into items
                    std::vector<std::string> items;
                    std::string item;
                    std::string pl = variables[array_name].value;
                    // remove the brackets from the pythonList
                    pl = std::regex_replace(pl, std::regex("^\\[|\\]$"), "");
                    std::stringstream ss(pl);

                    while (std::getline(ss, item, ',')){
                        // remove any whitespace before and after the items
                        item = std::regex_replace(item, std::regex("^\\s+|\\s+$"), "");
                        items.push_back(item);
                    }

                    // if the start is empty then set it to 0
                    if (start == ""){
                        start = "0";
                    }
                    // if the end is empty then set it to the size of the list
                    if (end == ""){
                        end = std::to_string(items.size());
                    }

                    // remove items after the end index
                    for (int i = std::stoi(end); i < items.size(); i++){
                        items.pop_back();
                    }

                    // remove items before the start index
                    for (int i = 0; i < std::stoi(start); i++){
                        items.erase(items.begin());
                    }

                    // join the items back into a string and add ", " between them except for the last item
                    std::string new_line = "";
                    for (int i = 0; i < items.size(); i++){
                        new_line += items[i];
                        if (i != items.size() - 1){
                            new_line += ", ";
                        }
                    }

                    // add brackets to the front and back of the result
                    new_line = "[" + new_line + "]";
                    return token("pythonList", new_line);
                }
            }
        }

        // if the array name is a variable and it is a pythonList get the whole pythonList
        if (variables.find(array_name) != variables.end()){
            
            if (variables[array_name].type == "pythonList"){
                // split the pythonList into items
                std::vector<std::string> items;
                std::string item;
                std::string pl = variables[array_name].value;
                // remove the brackets from the pythonList
                pl = std::regex_replace(pl, std::regex("^\\[|\\]$"), "");
                std::stringstream ss(pl);

                while (std::getline(ss, item, ',')){
                    items.push_back(item);
                }

                // std:: cout << "index:" << index.value << std::endl;

                // get the item at the index
                std::string i = items[std::stoi(index)];

                // remove any trailing whitespace
                i = std::regex_replace(i, std::regex("^\\s+|\\s+$"), "");

                // return the item as a token
                return token("number", i);
            }
        }

    }
    // if the token is a variable declaration
    if (tokens.type == "variableDeclaration"){
        // recursiveley assign the value of the variable 
        tokens.nest = new token(parser_helper(*tokens.nest, variables, all_functions));
        // cout the token.next
        return tokens;
    }
    // if the token is a function declaration
    if (tokens.type == "functionCall"){
        std::string function_name = tokens.value;
        std::map<std::string, token> new_variables;

        // if the function is len
        if (function_name == "len"){
            // get the value of the argument
            token arg = *tokens.nest;
            arg = parser_helper(arg, variables, all_functions);
            // if the argument is a string
            if (arg.type == "string"){
                // remove the quotes from the string
                std::string s = std::regex_replace(arg.value, std::regex("^\"|\"$"), "");
                // return the length of the string
                return token("number", std::to_string(s.length()));
            }
            // if the argument is a pythonList
            if (arg.type == "pythonList"){
                // split the pythonList into items
                std::vector<std::string> items;
                std::string item;
                std::string pl = arg.value;
                // remove the brackets from the pythonList
                pl = std::regex_replace(pl, std::regex("^\\[|\\]$"), "");
                std::stringstream ss(pl);

                // if the pythonList is empty return 0
                if (pl == ""){
                    return token("number", "0");
                }
                // if the pythonList is a single item return 1
                if (pl.find(",") == std::string::npos){
                    return token("number", "1");
                }

                while (std::getline(ss, item, ',')){
                    items.push_back(item);
                }

                // return the length of the pythonList
                return token("number", std::to_string(items.size()));
            }
        }

        for (int i = 0; i < all_functions.size(); i++){
            if (all_functions[i].name == tokens.value){
                // get the linked list of tokens from the function
                token* function_tokens = all_functions[i].arguments;
                
                // get the arguments from the function call
                token* arguments = new token(tokens);

                // add the arguments to the new_variables map
                while (arguments->nest != nullptr){
                    new_variables[function_tokens->value] = parser_helper(*arguments->nest, variables, all_functions);
                    // std:: cout << "function_tokens->value" << " " << new_variables[function_tokens->value].value << std::endl;
                    function_tokens = function_tokens->nest;
                    arguments = arguments->nest;
                }

            }
        }

        // print out the new variables
        // for (auto const& x : new_variables){
        //     std::cout << x.first << " = " << x.second.value << std::endl;
        // }
        // std:: cout << "print out the new variables" << std::endl;

        // print the arguments
        return new token(recursive_parser(all_functions, tokens.value, new_variables));
    }
    // if the token is an arithmetic expression
    else if (tokens.type == "arithmeticExpression"){
        // replace the variables with their values
        int i = 0;

        // print out the variables in the variables map
        // for (auto const& x : variables){
        //     std::cout << x.first << ": " << x.second.value << std::endl;
        // }


        std::string line = replace_variables(tokens.value, variables, i);


        // replace any functions with their values
        line = replace_functions(line, all_functions);

        // throw error if the line contains a letter and print out the line number
        if (std::regex_match(line, std::regex(".*[a-zA-Z_].*"))){
            std::cout << "Error: Line " << tokens.line << " invalid arithmetic expression" << std::endl;
            // std:: cout << tokens.value << std::endl;
            exit(1);
        }


        // solve the arithmetic expression
        if (i == 0)
            return token("number", expression_solver(line));
        else {
            token t("pythonList", array_addition(line, tokens.line));

            // remove all " " from the pythonList
            t.value = std::regex_replace(t.value, std::regex(" "), "");

            // add " " between each item in the pythonList
            t.value = std::regex_replace(t.value, std::regex(","), ", ");

            return t;   
            }
    }
    // if the token is a arrayAddition
    else if (tokens.type == "arrayAddition"){

        // replace the variables with their values
        std::string line = tokens.value;

        token t("pythonList", array_addition(line, tokens.line));

        // remove all " " from the pythonList
        t.value = std::regex_replace(t.value, std::regex(" "), "");

        // add " " between each item in the pythonList
        t.value = std::regex_replace(t.value, std::regex(","), ", ");

        return t;        
    }
    // if token is a print statement
    else if (tokens.type == "printStatement"){
        // lopp through the tokens nest and recursively call parser_helper
        // on each token to get the value of the print statement and save it to a string
        token *next = tokens.nest;
        std::string line = "";
        while (next != nullptr){
            // std :: cout << parser_helper(*next, variables, all_functions).value << std ::endl;
            line += parser_helper(*next, variables, all_functions).value + " ";
            next = next->nest;
        }
        // remove the last space
        line = line.substr(0, line.length() - 1);
        // return the print statement
        token t("printStatement", line);
        t.indent = tokens.indent;
        
        return t;
        

    }
    // if the token is a number
    else if (tokens.type == "number"){
        return tokens;
    }
    // if the token is an array
    else if (tokens.type == "pythonList"){
        // add brackets to the front and back
        token t("pythonList", "[" + tokens.value + "]");
        int i = 0;

        t.value = replace_variables_for_pyhon_list(t.value, variables);

        // remove all " " from the pythonList
        t.value = std::regex_replace(t.value, std::regex(" "), "");

        // add " " between each item in the pythonList
        t.value = std::regex_replace(t.value, std::regex(","), ", ");

        // replace "[, " with "["
        t.value = std::regex_replace(t.value, std::regex("\\[, "), "[");

        return t;   

    }
    // if the token is an identifier
    else if (tokens.type == "identifier"){
        // if the variable is in the map
        if (variables.find(tokens.value) != variables.end()){

            return variables[tokens.value];
        }
        // if the variable is not in the map
        else{
            std::cout << "Error: variable " << tokens.value << " is not defined" << std::endl;
            exit(1);
        }
    }
    // if the token is a string
    else if (tokens.type == "string"){
        return tokens;
    }
    return token("error", "error");
}


// recursive parser
token recursive_parser(std::vector<funct> &all_functions, std::string &funct_name, std:: map<std::string, token> &variables){
    std::vector<token> parseTree;
    std::stack<int> stack;
    stack.push(2147483647);
    std::string line;
    std::string item;
    int block_indent = 2147483647;
    token _return = token("None", "None");

    // get the funct with name funct_name out of the all_functions vector
    funct f;
    for (int i = 0; i < all_functions.size(); i++){
        if (all_functions[i].name == funct_name){
            f = all_functions[i];
        }
    }

    // set the body of the function to tokens
    std::vector<token> tokens = f.body;


    // loop through each token
    for (int i = 0; i < tokens.size(); i++){

        // if current indent is less than the block indent and token is not empty reset the block indent
        if (tokens[i].indent < block_indent && tokens[i].type != "empty"){
            block_indent = 2147483647;
        }

        // if the block indent less than or equal to the current indent continue
        if (block_indent <= tokens[i].indent){
           continue;
        }

        // if the indent is greater than the top of the stack and the token is not an else statement
        if (tokens[i].indent > stack.top() && tokens[i].type != "elseStatement"){
            continue;
        }

        // if the indent is equal than the top of the stack and the token is an else statement
        if (tokens[i].indent == stack.top() && tokens[i].type == "elseStatement"){
            stack.pop();
            continue;

        }
        else if (tokens[i].type == "elseStatement"){
            block_indent = tokens[i+1].indent;
            continue;
        }


        if(stack.top() > tokens[i].indent){
            // std:: cout<< tokens[i].line << std::endl;
            token t(parser_helper(tokens[i], variables, all_functions));

            if (t.type == "returnStatementResult"){
                for (int i = 0; i < parseTree.size(); i++){
                    std::cout << parseTree[i].value << std::endl;}
                // return the nested token
                return t.nest;
            }
            if (t.type == "ifStatementResult" && t.value == "false"){
                stack.push(tokens[i].indent);
            }
            if (t.type == "variableDeclaration") {variables[tokens[i].value] = *t.nest;}
            if (t.type == "printStatement") {
                parseTree.push_back(t);}
            
        }
        
    }

    // evaluate the print statements
    for (int i = 0; i < parseTree.size(); i++)
            std::cout << parseTree[i].value << std::endl;

    return _return;
}


std::vector<funct> split_functs(std::vector<token> tokens){
    std::vector<funct> functions;
    
    // loop through each token
    for (int i = 0; i < tokens.size(); i++){

        if (tokens[i].type == "functionDeclaration"){
            funct f;
            f.name = tokens[i].value;
            
            // set the arguments of the function
            f.arguments = tokens[i].nest;

            // store all the the tokens in the function body
            int current_indent = tokens[i].indent;
            int j = i + 1;
            while (tokens[j].indent > current_indent){
                // if it is another function declaration break
                if (tokens[j].type == "functionDeclaration"){
                    break;
                }
                f.body.push_back(tokens[j]);
                j++;
            }

            // add the function to the functions vector
            functions.push_back(f);

            // delete the function declaration and body from the tokens vector
            tokens.erase(tokens.begin() + i, tokens.begin() + j);

            i = -1;
        }
    }

    // remove all "empty" tokens
    for (int i = 0; i < tokens.size(); i++){
        if (tokens[i].type == "empty"){
            tokens.erase(tokens.begin() + i);
            i--;
        }
    }

    // add the rest to themain function
    funct main;
    main.name = "main";
    main.body = tokens;
    functions.push_back(main);


    // print the functions that are not named main
    // for (int i = 0; i < functions.size(); i++){
    //     if (functions[i].name != "main"){
    //         std::cout << "function " << functions[i].name << "(";
    //         token *next = functions[i].arguments;
    //         while (next != nullptr){
    //             std::cout << next->value << ", ";
    //             next = next->nest;
    //         }
    //         std::cout << ")" << std::endl;
    //         for (int j = 0; j < functions[i].body.size(); j++){
    //             std::cout << functions[i].body[j].value << std::endl;
    //         }
    //         std::cout << std::endl;
    //     }
    // }
    

    return functions;
}


// Python Interpreter in C++
int main(int argc, char *argv[]){
    std::vector<std::string> content = readFile(argv[1]);
    std::vector<token> tokens = lexer(content);
    // call split_functs to split the functions
    std::vector<funct> functions = split_functs(tokens);
    // create a map to store the variables
    std::map<std::string, token> variables;
    std::string funct_name = "main";
    // call recursive_parser to parse the functions
    recursive_parser(functions, funct_name, variables);

    return 0;
}
