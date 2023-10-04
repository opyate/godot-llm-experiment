extends Node2D


func _on_gd_example_position_changed(node, new_pos):
	print("The position of " + node.get_class() + " is now " + str(new_pos))
