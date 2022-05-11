"""
Print an error (using fancy coloring) and terminate
"""
def throw_error(error_str):
    print('\033[91mError!\033[0m', error_str)
    quit()

"""
Read a file in its entirety
"""
def read_whole_file(filename):
    s = ''
    with open(filename, 'r') as fp:
        s = fp.read()
    return s

"""
Write the given string to the specified file
"""
def write_to_file(filename, s):
    if s[-1] != '\n': # Ensure string ends with a newline to make things neater
        s += '\n'
    with open(filename, 'w') as fp:
        fp.write(s)

"""
Read a file, and convert it into a list of instruction names
"""
def split_name_file(filename, delim = '\n'):
    full_text = read_whole_file(filename)
    L = full_text.split(delim)
    for i in range(len(L)):
        L[i] = L[i].strip()
    return L


"""
A data-containing class for an Avida instruction
"""
class Instruction:
    def __init__(self, _index, _id, _char, _name):
        self.index = _index
        self.id = _id
        self.char = _char
        self.name = _name
    def __str__(self):
        return '[index: ' + str(self.index) + ', id: ' + str(self.id) + ', char: ' + \
                self.char + ', name: ' + self.name + ']'

"""
A converter between symbol- and name-based Avida organism types 
"""
class GenomeManipulator:
    def __init__(self, _filename = None): 
        self.index_map = {}
        self.id_map = {}
        self.char_map = {}
        self.name_map = {}
        if _filename is not None: 
            self.read_instruction_set_file(_filename)

    """ 
    Load all the information of an instruction set from file at `filename`
    """
    def read_instruction_set_file(self, filename):
        index_col_idx = 0
        id_col_idx = 1
        char_col_idx = 2
        name_col_idx = 3
        with open(filename, 'r') as fp:
            line = fp.readline()
            line = line.replace(',', '')
            line_parts = line.split()
            # Check for header, if first _is_ a digit, don't skip this line
            if(line_parts[0].replace(',','').isdigit()): 
                fp.seek(0)
                print('No header detected. Using default column indices:')
                print('index:', index_col_idx)
                print('id:', id_col_idx)
                print('char:', char_col_idx)
                print('name:', name_col_idx)
            # Else, header exists and we should handle the column order
            else: 
                for i in range(len(line_parts)):
                    part = line_parts[i].strip()
                    if part == 'index': 
                        index_col_idx = i
                    elif part == 'id':
                        id_col_idx = i
                    elif part == 'char':
                        char_col_idx = i
                    elif part == 'name': 
                        name_col_idx = i 
                    else: 
                        print('Found unknown column: ' + part)
                print('After parsing header, column indices are as follows:')
                print('index:', index_col_idx)
                print('id:', id_col_idx)
                print('char:', char_col_idx)
                print('name:', name_col_idx)

            for line in fp:
                line = line.replace(',', '')
                line_parts = line.split()
                inst_index = int(line_parts[index_col_idx])
                inst_id = int(line_parts[id_col_idx])
                inst_char = line_parts[char_col_idx]
                inst_name = line_parts[name_col_idx]
                inst = Instruction(inst_index, inst_id, inst_char, inst_name)
                self.index_map[inst_index] = inst
                self.id_map[inst_id] = inst
                self.char_map[inst_char] = inst
                self.name_map[inst_name] = inst
                print(inst)

    """
    Converts a string, `s`, of instruction symbols (one char each) into a string containing
    full instruction names (one per line)
    """
    def convert_chars_to_names(self, s):
        output_str = ''
        for c in s: 
            if c not in self.char_map:
                throw_error('Unknown symbol: ' + c)
            output_str += self.char_map[c].name + '\n'
        return output_str
    
    """
    Converts a file containing instruction names (one per line) into a symbol string
    """
    def convert_names_to_chars(self, name_list):
        output_str = ''
        for s in name_list:
            s = s.strip()
            if len(s) == 0: 
                continue 
            if s not in self.name_map:
                throw_error('Unknown name: ' + s)
            output_str += self.name_map[s].char
        return output_str

if __name__ == '__main__':
    # Create manipulator
    manipulator = GenomeManipulator('inst_set.txt')
    # Convert a symbol list into a name list, write to standard out
    print(manipulator.convert_chars_to_names('abcdefghijklmnopqrstuvwxyz'))
    # Convert a symbol list into a name list, write to file 
    write_to_file('converted_symbol_list.org', \
            manipulator.convert_chars_to_names('abcdefghijklmnopqrstuvwxyz'))
    # Load a name-based .org file
    name_list = split_name_file('ancestor_extended.org')
    # Convert name list to symbol list, write to standard out
    print(manipulator.convert_names_to_chars(name_list))
    # Convert name list to symbol list, write to file 
    write_to_file('converted_name_list.org', manipulator.convert_names_to_chars(name_list))
