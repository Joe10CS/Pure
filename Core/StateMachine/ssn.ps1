# Define the full path to your new ss.cli.exe executable.
# YOU MUST UPDATE THIS PATH to your specific build location.
$ssCliPath = "C:\work\ZivAv\Station4\Dev\OpenSource\StateSmith\StateSmith\src\StateSmith.Cli\bin\Debug\net8.0\StateSmith.cli.exe"

# Run the ss.cli tool with the '-hr run' command.
# This assumes you are in the same folder as the PlantUML (.puml) file.
& $ssCliPath run -hr --verbose