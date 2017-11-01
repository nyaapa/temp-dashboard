use v6;
use HTTP::UserAgent;

my $s = IO::Socket::INET.new(:localport(7339), :type(1), :reuse(1), :localhost<0.0.0.0>, :listen);

while my $conn = $s.accept()  {
	my $data = $conn.get();
	my $success = True;
	if my $temp = $data ~~ m/^<(<[0..9]>+)>MK\s*$/ {
	my $ua = HTTP::UserAgent.new();
	$ua.timeout = 10;

	my $request = HTTP::Request.new(:post<http://localhost:8123>);
	$request.add-content("
		insert into Temperature.Iceboat(
			EventTime,
			MegaKelvin
		)
		values
			(now(), $temp)
	");
	my $response = $ua.request($request);
	if $response.is-success {
		say "Updated " ~ time ~ " <= $temp";
	} else {
		warn "Update failed: $response.status-line";
		$success = False;
	}
	} else {
		warn "Strange data '$data'";
		$success = False;
	}

	$conn.put: $success ?? "ok" !! "nope";
	$conn.close();
}