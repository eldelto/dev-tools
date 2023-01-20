package main

import (
	"bufio"
	"fmt"
	"os"
	"strings"

	"github.com/eldelto/devtools"
)

const (
	blue  = "\033[32m"
	reset = "\033[0m"
)

func main() {
	dirs := os.Args[1:]
	hyperShell := devtools.NewHyperShell()
	for _, dir := range dirs {
		subShell, err := devtools.NewSubShell(dir)
		if err != nil {
			panic(err)
		}
		hyperShell.Append(subShell)
	}

	for {
		fmt.Print(">")
		reader := bufio.NewReader(os.Stdin)
		command, err := reader.ReadString('\n')
		if err != nil {
			panic(fmt.Errorf("failed to read user input: %w", err))
		}
		command = strings.Trim(command, "\r\n")

		results := hyperShell.Run(command)
		for _, result := range results {
			fmt.Printf("%s%s%s:\n%s\n", blue, result.Dir, reset, result.Result)
		}
	}
}
