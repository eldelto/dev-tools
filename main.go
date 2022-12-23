package main

import (
	"bufio"
	"bytes"
	"fmt"
	"os"
	"os/exec"
	"strings"
	"sync"
)

type SubShell struct {
	dir    string
	input  chan string
	output chan<- string
}

func shellWorker(s *SubShell) {
	for rawCmd := range s.input {
		cmdOut := bytes.Buffer{}

		cmd := exec.Command(rawCmd)
		cmd.Dir = s.dir
		cmd.Stdout = os.Stdout
		cmd.Stdin = os.Stdin

		if err := cmd.Run(); err != nil {
			s.output <- err.Error()
			continue
		}

		s.output <- cmdOut.String()
	}
}

func NewSubShell(path string, output chan<- string) (*SubShell, error) {
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
		dir:    path,
		input:  make(chan string, 1),
		output: output,
	}

	go shellWorker(&shell)

	return &shell, nil
}

func (s *SubShell) QueueCmd(cmd string) {
	s.input <- cmd
}

func (s *SubShell) Close() {
	close(s.input)
}

func mainOutputWorker(output <-chan string, waitGroup *sync.WaitGroup) {
	for msg := range output {
		fmt.Println(msg)
		waitGroup.Done()
	}
}

func main() {
	mainOut := make(chan string)
	defer close(mainOut)

	dirs := os.Args[1:]
	subShells := []*SubShell{}
	for _, dir := range dirs {
		subShell, err := NewSubShell(dir, mainOut)
		if err != nil {
			panic(err)
		}
		subShells = append(subShells, subShell)
	}

	waitGroup := sync.WaitGroup{}
	go mainOutputWorker(mainOut, &waitGroup)

	for {
		fmt.Print(">")
		reader := bufio.NewReader(os.Stdin)
		userInput, err := reader.ReadString('\n')
		if err != nil {
			panic("failed to read user input")
		}
		userInput = strings.Trim(userInput, "\r\n")

		for _, subShell := range subShells {
			waitGroup.Add(1)
			subShell.QueueCmd(userInput)
		}

		waitGroup.Wait()
	}
}
