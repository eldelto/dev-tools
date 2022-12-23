package main

import (
	"bufio"
	"bytes"
	"fmt"
	"os"
	"os/exec"
	"strings"
)

type SubShell struct {
	dir string
}

func NewSubShell(path string) (*SubShell, error) {
	file, err := os.Open(path)
	if err != nil {
		return nil, fmt.Errorf("Failed to open file while creating a sub shell: %w", err)
	}
	defer file.Close()

	info, err := file.Stat()
	if err != nil {
		return nil, fmt.Errorf("Failed to read file stats while creating a sub shell: %w", err)
	}

	if !info.IsDir() {
		return nil, fmt.Errorf("Failed to create a sub shell: '%s' is not a directory", path)
	}

	shell := SubShell{
		dir: path,
	}

	return &shell, nil
}

func (s *SubShell) Run(command string) (string, error) {
	buffer := bytes.Buffer{}

	cmd := exec.Command("fish", "-c", command)
	cmd.Dir = s.dir
	cmd.Stdout = &buffer

	if err := cmd.Run(); err != nil {
		return "", fmt.Errorf("Error for sub shell '%s': %w", s.dir, err)
	}

	return buffer.String(), nil
}

type ShellResult struct {
	Dir    string
	Result string
}

type SubShells struct {
	subShells []*SubShell
}

func NewSubShells() *SubShells {
	return &SubShells{[]*SubShell{}}
}

func (s *SubShells) Append(subShell *SubShell) {
	s.subShells = append(s.subShells, subShell)
}

func (s *SubShells) Broadcast(command string) ([]ShellResult, error) {
	results := []ShellResult{}

	for _, subShell := range s.subShells {
		result, err := subShell.Run(command)
		if err != nil {
			return nil, err
		}

		results = append(results, ShellResult{subShell.dir, result})
	}

	return results, nil
}

const (
	blue  = "\033[32m"
	reset = "\033[0m"
)

func main() {
	dirs := os.Args[1:]
	subShells := NewSubShells()
	for _, dir := range dirs {
		subShell, err := NewSubShell(dir)
		if err != nil {
			panic(err)
		}
		subShells.Append(subShell)
	}

	for {
		fmt.Print(">")
		reader := bufio.NewReader(os.Stdin)
		command, err := reader.ReadString('\n')
		if err != nil {
			panic(fmt.Errorf("failed to read user input: %w", err))
		}
		command = strings.Trim(command, "\r\n")

		results, err := subShells.Broadcast(command)
		if err != nil {
			panic(err)
		}

		for _, result := range results {
			fmt.Printf("%s%s%s:\n%s\n", blue, result.Dir, reset, result.Result)
		}
	}
}
