#!/usr/bin/env bash

# Global flags
global_flag_f_format=0
global_flag_s_format=0
global_flag_b_build=0
global_flag_r_rebuild=0
global_flag_e_execute=0
global_flag_v_verbose=0
global_value_target=""

#################################################################################

#
# Print usage.
#
function print_help()
{
    cat << EOF
    Use like this:

    ./bbuild.sh <flags> <target>

    flags:
    -f, --format        [f]ormat all source files with clang-formatter
    -s, --static        [s]tatic analysis all source files with clang-tidy
    -b, --build         [b]uild
    -r, --rebuild       [r]euild
    -e, --execute       [e]xecute
    -v, --verbose       [v]erbose

    targets:
     <target> is a positional argument. Either "app" of "test"
EOF

    return 0
}

################################################################################

#
# Print a fancy banner to separate sections visually.
#
function print_banner()
{
    if [[ global_flag_v_verbose -eq 1 ]]; then
        printf "\n================================================================================\n"
        printf "%s\n" "$*"
        printf "================================================================================\n\n"
    fi
}

################################################################################

#
# Print text a simple header to make it easier to spot.
#
function print_header()
{
    if [[ global_flag_v_verbose -eq 1 ]]; then
        printf "==-- %s --==\n" "$*"
    fi
}

################################################################################

#
# Format with clang-format
#
function func_format()
{
    print_banner "Formatting code"

    # -iname pattern: Returns true if the file’s name matches the provided shell 
    # pattern. The matching here is case insensitive.

    # xargs is a great command that reads streams of data from standard input, 
    # then generates and executes command lines; meaning it can take output of a 
    # command and passes it as argument of another command. If no command is specified, 
    # xargs executes echo by default. You many also instruct it to read data from a file 
    # instead of stdin.

    find . -iname '*.*pp' | grep --invert-match './build' | grep --invert-match './external' | xargs clang-format -i -style=file
}

################################################################################

#
# Static analysis with clang-tidy
#
function func_static()
{
    print_banner "Performing static analysis on code"
    print_header "Not implemented yet..."

    #find . -iname '*.*pp' | grep --invert-match './build' | grep --invert-match './external' | xargs clang-tidy -format-style=file -header-filter=. -p build -checks=-*,clang-analyzer-*,-clang-analyzer-cplusplus*
}

################################################################################

#
# Build with make/cmake
#
function func_build()
{
    print_banner "Building code"

    cmake -S . -B build -D TARGET_GROUP=$1
    cmake --build build --parallel `nproc`

}

################################################################################

#
# Rebuild with make/cmake
#
function func_rebuild()
{
    print_banner "Rebuilding code"

    rm -rf build
    func_build $1
}

################################################################################

#
# Execute the binary
#
function func_execute()
{
    print_banner "Executing code"

    ./build/main
}

################################################################################

#
# Gather all params passed to the script
#
function gather_params()
{
    while [[ $# -gt 0 ]]; do
        case $1 in
            -f | --format)
                global_flag_f_format=1
                shift
                ;;
            -s | --static)
                global_flag_s_format=1
                shift
                ;;
            -b | --build)
                global_flag_b_build=1
                shift
                ;;
            -r | --rebuild)
                global_flag_r_rebuild=1
                shift
                ;;
            -e | --execute)
                global_flag_e_execute=1
                shift
                ;;
            -v | --verbose)
                global_flag_v_verbose=1
                shift
                ;;
            -h | --help)
                print_help
                exit
                ;;
            *)
                global_value_target=$1
                shift
                ;;
        esac
    done

    # Allow only format or static analysis without specifying <target>
    #if [ global_flag_f_format -eq 0 ] && [ global_flag_s_format -eq 0 ]; then
    if [ "$global_flag_f_format" -eq 0 ] && [ "$global_flag_s_format" -eq 0 ]; then
        if [[ -z "$global_value_target" ]]; then
            echo "Please, define a <target>"
            print_help
            exit
        fi
    fi
}

################################################################################

#
# Execute script logic based on parameters
#
function execute_logic()
{
    if [[ global_flag_f_format -eq 1 ]]; then
        func_format
    fi
    if [[ global_flag_s_format -eq 1 ]]; then
        func_static
    fi
    if [[ global_flag_b_build -eq 1 ]]; then
        func_build "$global_value_target"
    fi
    if [[ global_flag_r_rebuild -eq 1 ]]; then
        func_rebuild "$global_value_target"
    fi
    if [[ global_flag_e_execute -eq 1 ]]; then
        func_execute
    fi
}

################################################################################

#
# main function
#
function main()
{
    # Gather params
    gather_params "$@"
    execute_logic "$global_value_target"
}


main "$@"