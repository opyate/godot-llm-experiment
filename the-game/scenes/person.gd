extends Label

signal say(something)

var natural_name


func _ready():
	natural_name = "Person %s" % [name.right(1)]
	say.connect(say_it)


func say_it(sender: Node, something: String):
	if natural_name in something:
		something = something.replace("%s: " % [natural_name], "")
		print("%s says '%s'" % [natural_name, something])
		speech_bubble(something)
		await get_tree().create_timer(0.1).timeout
		sender.emit_signal("next_please")
	else:
		speech_bubble("")


func speech_bubble(something):
	if something:
		show()
		text = something
	else:
		hide()
	
