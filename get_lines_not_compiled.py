import re
import enum
from typing import Dict, List, Optional, NamedTuple


class LinemarkerFlags(enum.Enum):
    NEW = enum.auto()
    RETURN = enum.auto()
    SYS_HEADER = enum.auto()
    EXTERN_C = enum.auto()


class LinemarkerData(NamedTuple):
    filename: str
    line_number: int
    flags: List[LinemarkerFlags]


class RangeTup(NamedTuple):
    start: int
    end: int


linemarker_re = re.compile(r'# (\d+) \"(.+)\"(.*)')

compiled_lines: Dict[str, List[int]] = {}


def parse_linemarker_str(linemarker_str: str) -> Optional:
    match = linemarker_re.match(linemarker_str)
    if match is None:
        return None

    # https://gcc.gnu.org/onlinedocs/cpp/Preprocessor-Output.html
    # for the syntax
    line_no = int(match.group(1))
    file_name = match.group(2)

    flag_str = match.group(3)
    flags = []
    if '1' in flag_str:
        flags.append(LinemarkerFlags.NEW)
    if '2' in flag_str:
        flags.append(LinemarkerFlags.RETURN)
    if '3' in flag_str:
        flags.append(LinemarkerFlags.SYS_HEADER)
    if '4' in flag_str:
        flags.append(LinemarkerFlags.EXTERN_C)

    return LinemarkerData(file_name, line_no, flags)


def read_file_lines(filename: str, encoding='iso-8859-15') -> List[str]:
    with open(filename, 'r', encoding=encoding) as f:
        lines = f.readlines()
    return lines


def add_compiled_line(filename: str, line_num: int):
    if filename in compiled_lines:
        # Add to old if not already
        if line_num not in compiled_lines[filename]:
            compiled_lines[filename].append(line_num)
        else:
            # Compiled line has already been added
            pass
    else:
        # Create new
        compiled_lines[filename] = [line_num]


def parse_preprocessed_lines(lines: List[str]):
    last_linemarker_data: Optional[LinemarkerData] = None
    virtual_line_number = 0

    for real_line_number, line in enumerate(lines, start=1):
        if line.startswith('# '):
            last_linemarker_data = parse_linemarker_str(line)
            if not last_linemarker_data:
                raise LookupError(f'Could not parse linemarker: {line}')
            virtual_line_number = last_linemarker_data.line_number

        if last_linemarker_data is None:
            raise KeyError(f'No linemarker data for line: {real_line_number} @ {line}')
        add_compiled_line(last_linemarker_data.filename, virtual_line_number)
        virtual_line_number += 1


def get_lines_not_compiled(filename_key: str, source_lines: List[str]) -> List[int]:
    lines_not_compiled: List[int] = []
    compiled_source_lines = compiled_lines[filename_key]
    for source_line_num, source_line in enumerate(source_lines, start=1):
        if source_line_num not in compiled_source_lines:
            # print(f'Not compiled {source_line_num}:{source_line.rstrip()}')
            lines_not_compiled.append(source_line_num)
    return lines_not_compiled


def ignore_single_unimportant_lines(not_compiled_ranges: List[RangeTup], source_lines: List[str]) -> List[RangeTup]:
    """
    The preprocessed output doesn't include preprocessor control flow (obviously) like #ifdef, #if
    and it sometimes skips empty lines. So we can go through the uncompiled line ranges and remove the ranges
    that only have these "unimportant" lines. Cuts down on false positives.
    """

    def should_ignore_source_line(l: str) -> bool:
        source_line_blank = l.strip() == ''
        source_line_preprocessor = l.lstrip().startswith('#')
        return source_line_blank or source_line_preprocessor

    def should_ignore_range(r: RangeTup) -> bool:
        return all(should_ignore_source_line(s_l) for s_l in source_lines[r.start - 1:r.end])

    striped_not_compiled_ranges = list(filter(should_ignore_range, not_compiled_ranges))
    return striped_not_compiled_ranges


def split_ranges(numbers: List[int]) -> List[RangeTup]:
    """
    Split up a list of numbers into a list of (start, end) tuples
    split_ranges([1, 2, 3, 4, 8, 10, 11])  # [(1, 4), (8, 8), (10, 11)]
    """
    numbers = sorted(set(numbers))  # remove duplicates and sort
    ranges: List[RangeTup] = []
    start: Optional[int] = None
    end: Optional[int] = None
    for number in numbers:
        if start is None or number != end + 1:
            # start a new range
            if start is not None:
                # store the current range
                ranges.append(RangeTup(start, end))
            start = number
            end = number
        else:
            # extend the current range
            end = number
    # store the last range
    ranges.append(RangeTup(start, end))
    return ranges


def main():
    # Need to add "-save-temps -C"
    filename_to_check = 'LcdButtons'

    preprocessed_lines = read_file_lines(f'{filename_to_check}.ii')
    parse_preprocessed_lines(preprocessed_lines)
    for filename, line_arr in compiled_lines.items():
        if 'platformio' not in filename:
            print(f'{filename}\t{line_arr}')

    filename_key = f'src/{filename_to_check}.cpp'
    source_lines = read_file_lines(filename_key)
    lines_not_compiled = get_lines_not_compiled(filename_key, source_lines)
    ranges_not_compiled = split_ranges(lines_not_compiled)
    ranges_not_compiled = ignore_single_unimportant_lines(ranges_not_compiled, source_lines)
    print('Lines not compiled:', ranges_not_compiled)


if __name__ == "__main__":
    main()
