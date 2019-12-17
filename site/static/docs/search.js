const menu_id = "docs_menu";
const filter_id = "search_input";

window.onload = function()
{
	UpdateActiveDoc(window.location.hash.substr(1));
};

function StringMatch4coderFuzzy(a, b)
{
	let match = true;
	let b_upper = b.toUpperCase();
	let a_substrings = a.toUpperCase().split(/[ _*]+/);
	let minimum_index = 0;

	for(let i = 0; i < a_substrings.length; ++i)
	{
		if(a_substrings[i].length > 0)
		{
			let index_of_substring = b_upper.indexOf(a_substrings[i], minimum_index);
			if(index_of_substring < 0 || index_of_substring < minimum_index)
			{
				match = false;
				break;
			}
			minimum_index = index_of_substring + a_substrings[i].length - 1;
		}
	}

	return match;
}

function SearchKeyDown(event)
{
	if(event.keyCode == 13)
	{
		event.preventDefault();
		let ul = document.getElementById(menu_id);
		let li = ul.getElementsByTagName("li");

		for (let i = 0; i < li.length; i++)
		{
			if(li[i].style.display == "")
			{
				UpdateActiveDoc(li[i].getElementsByTagName("a")[0].innerHTML);
	    		window.location.hash = li[i].getElementsByTagName("a")[0].innerHTML;
	    		break;
			}
		}
	}
}

function SearchKeyUp(event)
{
	let ul = document.getElementById(menu_id);
	let li = ul.getElementsByTagName("li");
	let input = document.getElementById(filter_id);
	let filter = input.value.toUpperCase();
	for(let i = 0; i < li.length; i++)
	{
		if(filter.length > 0)
		{
			let a = li[i].getElementsByTagName("a")[0];
			if(StringMatch4coderFuzzy(filter, a.innerHTML))
			{
				li[i].style.display = "";
			}
			else
			{
			    li[i].style.display = "none";
			}
		}
		else
		{
			li[i].style.display = "";
		}
	}
}

function UpdateActiveDoc(name)
{
	let active_hash = window.location.hash.substr(1);
	let new_hash = name;
	if(active_hash.length > 0)
	{
		document.getElementById(active_hash).classList.add("hidden");
	}
	document.getElementById(new_hash).classList.remove("hidden");
	document.getElementById(filter_id).focus();
}