1. Nalozimo sliko. Ce rezolucija ni 2^N*2^N, dodamo nicle 
2. Jemljemo bloke 8x8
3. Od vsakega pixela v bloku odstejemo 128
4. FDCT
	for (int u = 0; u < 8; u++)
	{
		for (int v = 0; v < 8; v++)
		{
			double c1, c2, vsota = 0;
			if (u == 0)
			{
				c1 = 1/sqrt(2);
			}
			else
			{
				c1 = 1;
			}
			if (v == 0)
			{
				c2 = 1/sqrt(2);
			}
			else
			{
				c2 = 1;
			}
			for (int x = 0; x < 8; x++)
			{
				for (int y = 0; y < 8; y++)
				{
					vsota += slika8x8[x][y] * cos...;
				}
			}
			izhod8x8[u][v] = 0.25 * c1 * c2* vsota;
		}
	}
5. FAKTOR STISKANJA
6. CIK-CAK
7. DATOTEKA
Dve pravili: