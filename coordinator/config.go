package main

import "os"

type Config struct {
	Port               string
	DBPath             string
	GoogleClientID     string
	GoogleClientSecret string
}

func LoadConfig() Config {
	port := os.Getenv("PORT")
	if port == "" {
		port = "8888"
	}
	if port[0] != ':' {
		port = ":" + port
	}

	dbPath := os.Getenv("DB_PATH")
	if dbPath == "" {
		dbPath = "coordinator.db"
	}

	return Config{
		Port:               port,
		DBPath:             dbPath,
		GoogleClientID:     os.Getenv("GOOGLE_CLIENT_ID"),
		GoogleClientSecret: os.Getenv("GOOGLE_CLIENT_SECRET"),
	}
}
