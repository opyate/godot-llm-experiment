extends Node2D

func _ready():
	$World.position.x = int(ProjectSettings.get_setting("display/window/size/viewport_width")) / 2
	$World.position.y = int(ProjectSettings.get_setting("display/window/size/viewport_height")) / 2
