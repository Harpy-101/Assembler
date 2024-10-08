One-Pass Assembler
## Overview

For more insight into the design choices, project goals, and reflections, see [REFLECTION.md](REFLECTION.md).

This project is a one-pass assembler designed to efficiently translate assembly language into machine code in a single pass. The assembler is capable of processing instructions, directives, and labels with minimal backtracking, making it both fast and effective for real-world applications.
**Key Features:**

- One-Pass Translation: The assembler processes the entire assembly code in a single pass, which reduces the need for multiple scans of the input file.
- Hash Tables: Utilizes hash tables for quick lookup of labels, instructions, and directives, significantly improving the performance.
- Error Handling: Built-in mechanisms to catch syntax errors, redefinitions, and incorrect addressing modes during translation.

## Project Structure

The assembler consists of several key modules, each responsible for a specific aspect of the translation process:

**1. Lexer**

The lexer is responsible for breaking down each line of the assembly code into tokens. It classifies tokens into different types such as instructions, registers, labels, directives, integers, strings, and commas. The lexer handles errors such as unknown tokens and prepares the token array for further processing.

Diagram:

```mermaid
    graph TD
        classDef Error stroke:#f00
        
        root[Assembler] --> lexer --> AST

        subgraph lexer[lexer.c]
        input[Line of code] 
        input --> |Broken into words |tokenize[tokenize]
        %% Token types
        Ttype{Token type?}
        Ttype --> |Instruction| T1[Process as an instruction]
        Ttype --> |Register| T2[Process as a register]
        Ttype --> |Label definition| T3[Process as label definition]
        Ttype --> |Directive| T4[Process as a directive]
        Ttype --> |Integer| T5[process as a integer]
        Ttype --> |String| T6[Process as a string]
        Ttype --> |Potential label| T7[Process as a label]
        Ttype --> |Comma| T8[Process as a comma] 
        Ttype --> |Unknown| T9[Error]
        tokenize --> Ttype
        
        %% Error for an unknown token 
        T9 --> Error[<i class="fa-solid fa-xmark"></i>]:::Error

        %% Creating the token and adding to array
        Create[Create token]
        T1 & T2 & T3 & T4 & T5 & T6 & T7 & T8 --> Create
        Create --> Add[Add to token array]
        
        %% Final array of tokens
        Add --> Tokens[Array of all tokens]
        end
```

**2. Abstract Syntax Tree (AST)**

The AST module translates the tokens generated by the lexer into a structured format. It differentiates between instructions, directives, and labels, processing each accordingly. This module also handles syntax errors, checking for excess code, and ensures labels are properly linked to their corresponding instructions or directives.

Diagram:

```mermaid

graph TD
    subgraph AST[ast.c]
    %% Node type and initial decision
    Ntype{Node type?}
    Ntype ---> |Instruction| N1[Process as an Instruction]
    Ntype ---> |Directive| N4[Process as a Directive]
    Ntype --> |Label| Label_scope

    %% Instruction processing
    N1 --> Build_instruction_node[Build an instruction node]
    Build_instruction_node --> Check_for_syntax_error_or_excess_code{Syntax error/excess code?}
    Check_for_syntax_error_or_excess_code --> |Yes| Syntax_error_or_excess_code[<i class="fa-solid fa-xmark"></i>]:::Error
    Check_for_syntax_error_or_excess_code --> |No| Add_to_node_list[Add to node list]

    %% Directive processing
    N4 --> Dtype{Directive type?}
    Dtype --> |data| D1[Process as a data directive] --> Check_for_syntax_error_or_excess_code
    Dtype --> |string| D2[Process as a string directive] --> Check_for_syntax_error_or_excess_code
    Dtype --> |extern/entry| Check_for_syntax_error_or_excess_code --> |extern/entry| Check_for_redefinition{Is this a redefinition?}
    Check_for_redefinition --> |Yes| Redefinition_Error[<i class="fa-solid fa-xmark"></i>]:::Error
    Check_for_redefinition --> |No| Add_to_directive_table[Add to directive table]
    Add_to_directive_table --> Add_to_node_list

    %% Label processing
    Label_scope{Combined with?}
    Label_scope --> |Instruction| N1
    Label_scope --> |Directive| N4

    Add_to_node_list --> End_of_file{End of file?}
    End_of_file --> |No| Get_next_line[Tokenize the next line]
    End_of_file --> |Yes| Start_translating[Start translating]

    end
```

**3. Translation Unit**

This module takes the structured AST and translates it into machine code. It handles label references, adding them to an unresolved list if they cannot be immediately resolved. The module processes each instruction and directive, ensuring the correct addressing modes are used and handling errors if not.

Diagram:

```mermaid

graph TD
    subgraph translation_unit[translation_unit.c]
    Node_type{Node type?}
    Node_type --> |Instruction| AST_instruction[Translate as an instruction]
    Node_type --> |Directive| AST_directive[Translate as a directive]
    Node_type --> |Label| AST_label[Translate as a label]

    %% instruction processing
    AST_instruction --> Encountered_a_label{Found label reference?}
    Encountered_a_label --> |Yes| Add_to_unresolved_list[Add to unresolved list]
    Encountered_a_label --> |No| Wrong_addressing_mode

    AST_instruction --> Wrong_addressing_mode{Wrong addressing mode?}
    Wrong_addressing_mode --> |Yes| Addressing_mode_error[<i class="fa-solid fa-xmark"></i>]:::Error
    Wrong_addressing_mode --> |No| Add_to_code_list[Add the words to code list]

    %% directive processing
    AST_directive --> |data/string| Translate_individual_chars_ints[Translate each char/int]
    AST_directive --> |extern/entry| Add_to_directive_list[Add to the directive list]

    Translate_individual_chars_ints --> Add_to_data_list[Add to data list]

    %% label processing
    AST_label --> |Instruction| AST_instruction
    AST_label --> |Directive| AST_directive

    %% Adding to symbol table after translation
    Originally_a_label{Called by a label?}
    Originally_a_label --> |Yes| Is_label[Add to the symbol table]
    Originally_a_label --> |No| Not_a_label[Do nothing]

    Add_to_data_list --> Originally_a_label
    Add_to_code_list --> Originally_a_label

    end
```

**4. File GeneratioN**

Once all the translation and processing are complete, the final module generates the output files. This includes patching word addresses, resolving unresolved labels, and creating the object (.ob), external (.ext), and entry (.ent) files. If any errors are encountered during the process, the assembler aborts the file creation to prevent corrupt output.

Diagram:

```mermaid

graph TD
    subgraph file_generation[file_generator.c]
    Patch_address[Patch word address] --> Resolve_unresolved_labels_list
    Resolve_unresolved_labels_list[Resolve unresolved labels] --> Any_errors_along_the_way

    Any_errors_along_the_way{Any errors along the way?}
    Any_errors_along_the_way --> |Yes| Abort[Create nothing]
    Any_errors_along_the_way --> |No| Create_files[Create ob, ent, and ext files]

    end
```

