vergeclass "Button" do
	function Button:__init(...)
		self.release = false
		self.inputs = { ... }
	end
	
	Button._property('pressed',
			function(self)
				local pressed = false
				for i, inp in ipairs(self.inputs) do
					if inp.pressed then
						pressed = true
						break
					end
				end
				
				if not pressed then
					if self.release then
						self.release = false
					end
					return false
				elseif not self.release then
					return true
				end
			end,
			
			function(self, value)
				if not value then
					self.release = true
				end
			end)
	
	function Button:Unpress()
		self.pressed = false
	end
	
	function Button:__tostring()
        return "button"
		--return "button " .. ObjectAttributesToString(self)
	end
end
