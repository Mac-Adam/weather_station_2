<?php
	$link = new mysqli("sensitive","data","","");
	$sql = <<<SQL
		INSERT INTO `odczyt`
		VALUES(
			NULL,
			NULL,
			'{$_POST["id_stacji"]}',
			'{$_POST["predkosc_wiatru"]}',
			'{$_POST["predkosc_porywow"]}',
			'{$_POST["kierunek_wiatru"]}',
			'{$_POST["std_div_kierunku"]}',
			'{$_POST["opad"]}',	
			'{$_POST["pm_sp_1"]}',
			'{$_POST["pm_sp_2_5"]}',
			'{$_POST["pm_sp_10"]}',
			'{$_POST["temperatura"]}',
			'{$_POST["temp_odczu"]}',
			'{$_POST["cisnienie"]}',
			'{$_POST["wilgotnosc"]}'
		)
	SQL;
	$result = mysqli_query($link,$sql);
	if($result){
		echo "succes";
	}	
	else{
		echo $sql;
	}
?>