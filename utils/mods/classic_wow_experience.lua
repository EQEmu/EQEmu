--Mod file to demo changing the experience tables
--In this case I used some old wow tables (roughly it's not 100%)

function GetRequiredAAExperience(e)
	e.level = 51;
	return GetEXPForLevel(e);
end

function GetExperienceForKill(e)
	local ML = e.other:GetLevel();
	local CL = e.self:GetLevel();
	
	if(ML > CL) then
		local lmod = (ML - CL) * 0.05;
		if(lmod > 1.0) then
			lmod = 1.0;
		end
		e.ReturnValue = BaseXP(ML) * (1 + lmod);
	elseif(ML < CL) then	
		local lmod = (CL - ML) * 0.05;
		if(lmod > 1.0) then
			lmod = 1.0;
		end
		e.ReturnValue = BaseXP(ML) * (1 - lmod);
	else
		e.ReturnValue = BaseXP(ML);
	end
	
	e.IgnoreDefault = true;
	return e;
end

function BaseXP(L)
	local base = L * 5;
	
	if(L < 60) then
		base = base + 45;
	elseif(L < 70) then
		base = base + 235;
	elseif(L < 80) then
		base = base + 580;
	else
		base = base + 1875;
	end
	
	return base;
end

function GetEXPForLevel(e)
	local exp_table = {
		0,
		400,
		900,
		1400,
		2100,
		2800,
		3600,
		4500,
		5400,
		6500,
		7600,
		8700, 
		9800, 
		11000,
		12300,
		13600,
		15000,
		16400,
		17800,
		19300,
		20800,
		22400,
		24000,
		25500,
		27200,
		28900,
		30500,
		32200,
		33900,
		36300,
		38800,
		41600,
		44600,
		48000,
		51400,
		55000,
		58700,
		62400,
		66200,
		70200,
		74300,
		78500,	
		82800,	
		87100,	
		91600,	
		96300,	
		101000,
		105800,
		110700,
		115700,
		120900,
		126100,
		131500,
		137000,
		142500,
		148200,
		154000,
		159900,
		165800,
		172000,
		290000,
		317000,
		349000,
		386000,
		428000,
		475000,
		527000,
		585000,
		648000,
		717000,
		1523800,
		1539000,
		1555700,
		1571800,
		1587900,
		1604200,
		1620700,
		1637400,
		1653900,
		1670800,
		1670800,
		1670800,
		2121500,
		2669000,
		3469000,
		4583000,
		13000000,
		15080000,
		22600000,
		27300000,
		32800000
	};
	
	if(e.level < 1) then
		e.ReturnValue = 0;
		e.IgnoreDefault = true;
		return e;
	end
	
	if(e.level > 91) then
		e.ReturnValue = exp_table[91];
		e.IgnoreDefault = true;
		return e;
	end

	e.ReturnValue = exp_table[e.level];
	e.IgnoreDefault = true;
	return e;
end
