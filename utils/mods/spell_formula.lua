function CalcSpellEffectValue_formula(e)
	--THE FUNCTIONALITY OF THIS IS FOR ADDING CUSTOM SPELL FORMULAS WITHOUT INTERFERING WITH EXISTING FORMULAS
	if (e.formula >= 100000) then
		e.IgnoreDefault = true;
	else 
		e.IgnoreDefault = false;
		return e;
	end

	local result = 0
	local updownsign = 1
	local ubase = e.base_value
	local formula = e.formula
	local base_value = e.base_value
	local max_value = e.max_value
	local caster_level = e.caster_level
	local spell_id = e.spell_id
	local ticsremaining = e.ticsremaining
	
	if (ubase < 0) then
		ubase = 0 - ubase;
	end
	
	if (max_value < base_value and max_value ~= 0) then
		--values are calculated down
		updownsign = -1;
	else
		--values are calculated up
		updownsign = 1;
	end
	
	--ADD FORMULAS HERE NO NEED TO TOUCH OTHER CODE UNLESS YOU KNOW WHAT YOU ARE DOING!!!
	--FORMULA NUMBERS 100,000+++ are considered "custom" and other spells will use normal logic from source!!!
	local switch = {
		[100000] = function ()
			result = updownsign * (ubase + (e.self:GetSTR()/2)); --Half strength added to damage!
		end,
		[100001] = function ()
			result = (updownsign * (ubase + (caster_level * 2))) * math.random(1,caster_level); -- multiplies final damage randomly from 1 up to caster level!
		end,
	}
	
	eq.log_spells(
			string.format("[Mob::CalcSpellEffectValue_formula] spell [%i] formula [%i] base [%i] max [%i] lvl [%i] Up/Down [%i]",
					e.spell_id,
					e.formula,
					e.base_value,
					e.max_value,
					e.caster_level,
					updownsign
			)
	);
	
	local f = switch[e.formula]
	if (f) then
		f()
	else
		result = ubase; --DEFAULT YOU MAY CHANGE THIS TO WHATEVER YOU WISH BUT WILL ONLY CHANGE CUSTOM FORMULAS above 100,000 THAT AREN'T IN THE TABLE ABOVE!
	end
	
	local oresult = result;
	
	--now check result against the allowed maximum
	if (max_value ~= 0) then
		if (updownsign == 1) then
			if (result > max_value) then
				result = max_value;
			end
		else
			if (result < max_value) then
				result = max_value;
			end
		end
	end
	
	--if base is less than zero, then the result need to be negative too
	if (base_value < 0 and result > 0) then
		result = result * -1;
	end
	
	eq.log_spells(
			string.format("[Mob::CalcSpellEffectValue_formula] Result: [%i] (orig [%i]) cap [%i]",
					result,
					oresult,
					max_value
			)
	);
	
	e.ReturnValue = result;
	return e;
end
	